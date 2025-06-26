export module hu.db.local.impl.RocksDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.local.LocalDBType;

import <rocksdb/db.h>;
import <rocksdb/options.h>;
import <rocksdb/utilities/transaction_db.h>;

import "hu/Core.hpp";


namespace hu {

using ParamType      = AString;
using TableHandleMap = std::map<LocalDBTableName, rocksdb::ColumnFamilyHandle*>;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

ParamType to_param(
    const Buffer& buffer
)
{
    return ParamType( buffer.cbegin(), buffer.cend() );
}

ParamType to_param(
    const String& str
)
{
    return util::to_utf8( str );
}

String to_str(
    const rocksdb::Status& status
)
{
    return util::format_str( _T( "ErrCode = {}, ErrMsg = {}" ),
        static_cast<UInt32>( status.code() ), util::to_str( status.ToString() ) );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////
 
class RocksDBTrans final : public LocalDBTransBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    RocksDBTrans(
        const LocalDBConfigInfo&    config,
        const LocalDBCheckRollback& check_rollback,
        rocksdb::Transaction* const impl,
        const TableHandleMap&       table_handles
    ) :
        LocalDBTransBase ( config, check_rollback ),
        impl_            ( impl ),
        table_handles_   ( table_handles )
    {
    }

    ~RocksDBTrans()
    {
        if ( impl_ )
        {
            if ( check_rollback_ && check_rollback_() )
            {
                impl_->Rollback();
            }
            else
            {
                const auto status = impl_->Commit();
                if ( status.ok() == false )
                {
                    impl_->Rollback();

                    HU_LOG_ERROR( kLocalDB, _T( "트랜잭션 커밋 실패 (Table = {}, {})" ),
                        config_.db, to_str( status ) );
                }
            }

            util::delete_ptr( impl_ );
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RocksDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Write(
        const LocalDBTableName& table,
        const LocalDBId&        id,
        const Buffer&           buffer
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_table_handle( table );
            if ( handle )
            {
                const auto status = impl_->Put( handle, id, to_param( buffer ) );
                if ( status.ok() )
                    return true;

                HU_LOG_ERROR( kLocalDB, _T( "디비 쓰기 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table ), to_str( id ), to_str( status ) );
            }

            impl_->Rollback();
            util::delete_ptr( impl_ );
        }

        return false;
    }

    virtual bool Read(
        const LocalDBTableName& table,
        const LocalDBId&        id,
        Buffer&                 buffer
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_table_handle( table );
            if ( handle )
            {
                ParamType value;

                const auto status = impl_->Get( read_options_, handle, id, &value );
                if ( status.ok() )
                {
                    buffer.assign( value.begin(), value.end() );
                    return true;
                }

                if ( status.IsNotFound() )
                    return false;

                HU_LOG_ERROR( kLocalDB, _T( "디비 읽기 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table ), to_str( id ), to_str( status ) );
            }

            impl_->Rollback();
            util::delete_ptr( impl_ );
        }

        return false;
    }

    virtual bool Delete(
        const LocalDBTableName& table,
        const LocalDBId&        id
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_table_handle( table );
            if ( handle )
            {
                const auto status = impl_->Delete( handle, id );
                if ( status.ok() )
                    return true;

                if ( status.IsNotFound() )
                    return false;

                HU_LOG_ERROR( kLocalDB, _T( "디비 삭제 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table ), to_str( id ), to_str( status ) );
            }

            impl_->Rollback();
            util::delete_ptr( impl_ );
        }

        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: private (RocksDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    

private:
    rocksdb::ColumnFamilyHandle* find_table_handle(
        const LocalDBTableName& table,
        const SrcLocation       loc = SrcLocation::current()
    ) const
    {
        const auto it = table_handles_.find( table );
        if ( it != table_handles_.end() )
            return it->second;

        util::log_error( loc, kLocalDB, _T( "테이블 찾기 실패 (Table = {})" ),
            to_str( table ) );
        return nullptr;
    }

private:
    rocksdb::Transaction* impl_ { nullptr };
    rocksdb::ReadOptions  read_options_;
    const TableHandleMap& table_handles_;
};

export class RocksDB final : public LocalDBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit RocksDB(
        const LocalDBConfigInfo& config
    ) :
        LocalDBBase( config )
    {
    }

    ~RocksDB()
    {
        if ( impl_ )
        {
            for ( const auto& [ table, handle ] : table_handles_ )
            {
                if ( const auto status = impl_->DestroyColumnFamilyHandle( handle ); status.ok() == false )
                    HU_LOG_ERROR( kLocalDB, _T( "Table handle 해제 실패 (Table = {}, {})" ), to_str( table ), to_str( status ) );
            }

            util::delete_ptr( impl_ );
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Open() override
    {
        rocksdb::Options options;
        {
            options.create_if_missing              = true;
            options.create_missing_column_families = true;
        }

        rocksdb::TransactionDBOptions trans_db_options;

        ParamType db_name;
        if ( config_.dir.empty() == false )
            db_name = to_param( util::format_str( _T( "{}/{}" ), config_.dir, config_.db ) );
        else
            db_name = to_param( config_.db );

        std::vector<rocksdb::ColumnFamilyDescriptor> table_descs;
        {
            std::vector<ParamType> table_names;
            const auto status = rocksdb::DB::ListColumnFamilies( options, db_name, &table_names );
            if ( ( status.ok() == false ) && ( status.IsNotFound() == false ) && ( status.IsPathNotFound() == false ) )
            {
                HU_LOG_ERROR( kLocalDB, _T( "테이블 목록 읽기 실패 (DB = {}, {})" ), config_.db, to_str( status ) );
                return false;
            }

            if ( table_names.empty() )
                table_names.push_back( rocksdb::kDefaultColumnFamilyName );

            auto tables = config_.tables;
            tables.insert( table_names.begin(), table_names.end() );

            for ( const auto& table : tables )
                table_descs.emplace_back( table, rocksdb::ColumnFamilyOptions() );
        }
        std::vector<rocksdb::ColumnFamilyHandle*> table_handles;

        const auto status = rocksdb::TransactionDB::Open( options, trans_db_options, db_name, table_descs, &table_handles, &impl_ );
        if ( ( status.ok() == false ) || ( impl_ == nullptr ) )
        {
            HU_LOG_ERROR( kLocalDB, _T( "디비 열기 실패 (DB = {}, {})" ), config_.db, to_str( status ) );
            return false;
        }

        for ( auto i = 0; i < table_handles.size(); ++i )
        {
            const auto& table = table_descs[ i ].name;

            auto* const table_handle = table_handles[ i ];
            if ( table_handle == nullptr )
            {
                HU_LOG_ERROR( kLocalDB, _T( "테이블 핸들 생성 실패 (Table = {})" ), to_str( table ) );
                return false;
            }

            table_handles_[ table ] = table_handle;
        }

        return true;
    }

    virtual LocalDBTransImpl CreateTrans(
        const LocalDBCheckRollback& check_rollback
    ) override
    {
        rocksdb::WriteOptions options;

        auto* const trans_impl_ = impl_->BeginTransaction( options );
        if ( trans_impl_ == nullptr )
        {
            HU_LOG_ERROR( kLocalDB, _T( "트랜잭션 시작 실패 (DB = {})" ),
                config_.db );
            return nullptr;
        }

        return std::make_unique<RocksDBTrans>( config_, check_rollback, trans_impl_, table_handles_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    rocksdb::TransactionDB* impl_ { nullptr };
    TableHandleMap          table_handles_;
};

} // hu
