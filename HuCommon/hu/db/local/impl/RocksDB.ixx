export module hu.db.local.impl.RocksDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <filesystem>;

import hu.db.DBType;

import <rocksdb/db.h>;
import <rocksdb/options.h>;
import <rocksdb/utilities/transaction_db.h>;

import "hu/Core.hpp";


namespace hu {

using TableHandleMap = std::map<DBTableName, rocksdb::ColumnFamilyHandle*>;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

String to_str(
    const rocksdb::Status& status
)
{
    return util::format_str( _T( "ErrCode = {}, ErrMsg = {}" ),
        static_cast<UInt32>( status.code() ), to_str( status.ToString() ) );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////
 
class RocksDBTrans final : public DBTransBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    RocksDBTrans(
        const DBConfigInfo&         config,
        const DBRollback&           rollback,
        rocksdb::Transaction* const impl,
        const TableHandleMap&       table_handles
    ) :
        DBTransBase    ( config, rollback ),
        impl_          ( impl ),
        table_handles_ ( table_handles )
    {
    }

    ~RocksDBTrans()
    {
        if ( impl_ )
        {
            if ( rollback_ && rollback_() )
            {
                impl_->Rollback();
            }
            else
            {
                const auto status = impl_->Commit();
                if ( status.ok() == false )
                {
                    impl_->Rollback();

                    HU_LOG_ERROR( kDB, _T( "트랜잭션 커밋 실패 ({}, {})" ),
                        config_.ToStr(), to_str( status ) );
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
        const DBTableName& table_name,
        const DBId&        id,
        const Buffer&      buffer
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_handle( table_name );
            if ( handle )
            {
                const auto status = impl_->Put( handle, id, to_param( buffer ) );
                if ( status.ok() )
                    return true;

                HU_LOG_ERROR( kDB, _T( "디비 쓰기 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( status ) );
            }

            impl_->Rollback();
            util::delete_ptr( impl_ );
        }

        return false;
    }

    virtual bool Read(
        const DBTableName& table_name,
        const DBId&        id,
        Buffer&            buffer
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_handle( table_name );
            if ( handle )
            {
                DBParamType param;

                const auto status = impl_->Get( read_options_, handle, id, &param );
                if ( status.ok() )
                {
                    to_buffer( param, buffer );
                    return true;
                }

                if ( status.IsNotFound() )
                    return false;

                HU_LOG_ERROR( kDB, _T( "디비 읽기 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( status ) );
            }

            impl_->Rollback();
            util::delete_ptr( impl_ );
        }

        return false;
    }

    virtual bool Delete(
        const DBTableName& table_name,
        const DBId&        id
    ) override
    {
        if ( impl_ )
        {
            auto handle = find_handle( table_name );
            if ( handle )
            {
                const auto status = impl_->Delete( handle, id );
                if ( status.ok() )
                    return true;

                if ( status.IsNotFound() )
                    return false;

                HU_LOG_ERROR( kDB, _T( "디비 삭제 실패 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( status ) );
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
    rocksdb::ColumnFamilyHandle* find_handle(
        const DBTableName& table_name,
        const SrcLocation  loc = SrcLocation::current()
    ) const
    {
        const auto it = table_handles_.find( table_name );
        if ( it != table_handles_.end() )
            return it->second;

        util::log_error( loc, kDB, _T( "테이블 찾기 실패 (Table = {})" ),
            to_str( table_name ) );
        return nullptr;
    }

private:
    rocksdb::Transaction* impl_ { nullptr };
    rocksdb::ReadOptions  read_options_;
    const TableHandleMap& table_handles_;
};

export class RocksDB final : public DBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit RocksDB(
        const DBConfigInfo& config
    ) :
        DBBase( config )
    {
    }

    ~RocksDB()
    {
        if ( impl_ )
        {
            for ( const auto& [ table, handle ] : table_handles_ )
            {
                if ( const auto status = impl_->DestroyColumnFamilyHandle( handle ); status.ok() == false )
                {
                    HU_LOG_ERROR( kDB, _T( "테이블 핸들 해제 실패 (Table = {}, {})" ),
                        to_str( table ), to_str( status ) );
                }
            }

            util::delete_ptr( impl_ );
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Connect() override
    {
        rocksdb::Options options;
        {
            options.create_if_missing              = true;
            options.create_missing_column_families = true;
        }

        rocksdb::TransactionDBOptions trans_db_options;

        DBParamType db_name;
        if ( config_.dir.empty() == false )
        {
            const String dir_path { util::format_str( _T( "{}/{}/" ), util::get_cur_path_str(), config_.dir ) };
            if ( std::filesystem::exists( dir_path ) == false )
                std::filesystem::create_directory( dir_path );

            db_name = to_param( util::format_str( _T( "{}/{}" ), config_.dir, config_.db ) );
        }
        else
        {
            db_name = to_param( config_.db );
        }

        std::vector<rocksdb::ColumnFamilyDescriptor> table_descs;
        {
            std::vector<DBParamType> table_names;
            const auto status = rocksdb::DB::ListColumnFamilies( options, db_name, &table_names );
            if ( ( status.ok() == false ) && ( status.IsNotFound() == false ) && ( status.IsPathNotFound() == false ) )
            {
                HU_LOG_ERROR( kDB, _T( "테이블 목록 읽기 실패 ({}, {})" ),
                    config_.ToStr(), to_str( status ) );
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
            HU_LOG_ERROR( kDB, _T( "디비 열기 실패 ({}, {})" ),
                config_.ToStr(), to_str( status ) );
            return false;
        }

        for ( auto i = 0; i < table_handles.size(); ++i )
        {
            const auto& table = table_descs[ i ].name;

            auto* const table_handle = table_handles[ i ];
            if ( table_handle == nullptr )
            {
                HU_LOG_ERROR( kDB, _T( "테이블 핸들 생성 실패 (Table = {})" ),
                    to_str( table ) );
                return false;
            }

            table_handles_[ table ] = table_handle;
        }

        return true;
    }

    virtual DBTransImpl CreateTrans(
        const DBRollback& rollback
    ) override
    {
        rocksdb::WriteOptions options;

        auto* const trans_impl_ = impl_->BeginTransaction( options );
        if ( trans_impl_ == nullptr )
        {
            HU_LOG_ERROR( kDB, _T( "트랜잭션 시작 실패 ({})" ),
                config_.ToStr() );
            return nullptr;
        }

        return std::make_unique<RocksDBTrans>( config_, rollback, trans_impl_, table_handles_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    rocksdb::TransactionDB* impl_ { nullptr };
    TableHandleMap          table_handles_;
};

} // hu
