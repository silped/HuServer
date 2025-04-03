export module hu.db.local.impl.RocksDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.local.LocalDBConfig;
import hu.db.local.LocalDBBase;
import hu.db.local.LocalDBType;
import hu.log.Log;
import hu.Type;
import hu.Util;

import <rocksdb/db.h>;
import <rocksdb/options.h>;
import <rocksdb/utilities/transaction_db.h>;

import "hu/db/local/LocalDBMacro.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

using TransImplType = rocksdb::Transaction;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

String get_error_str(
    const rocksdb::Status& status
)
{
    return util::get_format_str( _T( "ErrCode = {}, ErrMsg = {}" ),
        static_cast<UInt32>( status.code() ), util::astr_to_wstr( status.ToString() ) );
}


// 로컬 디비 트랜잭션 구현체를 위한 기반 클래스
class RocksDBTrans final : public LocalDBTransBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    RocksDBTrans(
        const LocalDBConfigInfo&    config,
        const LocalDBCheckRollback& check_rollback,
        TransImplType* const        impl
    ) :
        LocalDBTransBase { config, check_rollback },
        impl_            { impl }
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

                    HU_LOCALDB_ERROR( _T( "트랜잭션 커밋 실패 (DBName = {}, {})" ),
                        config_.name, get_error_str( status ) );
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
        const LocalDBKey& key,
        const Buffer&     buffer
    ) override
    {
        const auto status = impl_->Put( to_param( key ), to_param( buffer ) );
        if ( status.ok() == false )
        {
            impl_->Rollback();
            util::delete_ptr( impl_ );

            HU_LOCALDB_ERROR( _T( "디비 쓰기 실패 (DBName = {}, Key = {}, {})" ),
                config_.name, key, get_error_str( status ) );
            return false;
        }

        return true;
    }

    virtual bool Read(
        const LocalDBKey& key,
        Buffer&           buffer
    ) override
    {
        ParamType read_value;
        const auto status = impl_->Get( read_options_, to_param( key ), &read_value );
        if ( status.ok() == false )
        {
            if ( status.IsNotFound() == false )
            {
                util::delete_ptr( impl_ );

                HU_LOCALDB_ERROR( _T( "디비 읽기 실패 (DBName = {}, Key = {}, {})" ),
                    config_.name, key, get_error_str( status ) );
            }
            return false;
        }

        buffer.assign( read_value.begin(), read_value.end() );
        return true;
    }

    virtual bool Delete(
        const LocalDBKey& key
    ) override
    {
        const auto status = impl_->Delete( to_param( key ) );
        if ( status.ok() == false )
        {
            impl_->Rollback();
            util::delete_ptr( impl_ );

            HU_LOCALDB_ERROR( _T( "디비 삭제 실패 (DBName = {}, Key = {}, {})" ),
                config_.name, key, get_error_str( status ) );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: private (RocksDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using ParamType = std::string;

private:
    // 키를 파라미터로 변환한다.
    static ParamType to_param(
        const LocalDBKey& key
    )
    {
        return util::wstr_to_astr( key );
    }

    // 버퍼를 파라미터로 변환한다.
    static ParamType to_param(
        const Buffer& buffer
    )
    {
        return ParamType( buffer.cbegin(), buffer.cend() );
    }

private:
    TransImplType*       impl_ { nullptr };
    rocksdb::ReadOptions read_options_;
};

// RocksDB를 사용해서 구현한 로컬 디비 클래스
export class RocksDB final : public LocalDBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit RocksDB(
        const LocalDBConfigInfo& config
    ) :
        LocalDBBase { config }
    {
    }

    ~RocksDB()
    {
        util::delete_ptr( impl_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Open() override
    {
        rocksdb::Options options;
        {
            options.create_if_missing = true;
        }

        rocksdb::TransactionDBOptions trans_db_options;

        std::string db_name;
        if ( config_.dir.empty() == false )
            db_name = util::wstr_to_astr( util::get_format_str( _T( "{}/{}" ), config_.dir, config_.name ) );
        else
            db_name = util::wstr_to_astr( config_.name );

        const auto status = rocksdb::TransactionDB::Open( options, trans_db_options, db_name, &impl_ );
        if ( ( status.ok() == false ) || ( impl_ == nullptr ) )
        {
            HU_LOCALDB_ERROR( _T( "디비 열기 실패 (DBName = {}, {})" ),
                config_.name, get_error_str( status ) );
            return false;
        }

        return true;
    }

    virtual LocalDBTransPtr CreateTrans( const LocalDBCheckRollback& check_rollback ) override
    {
        rocksdb::WriteOptions options;

        auto* const trans_impl_ = impl_->BeginTransaction( options );
        if ( trans_impl_ == nullptr )
        {
            HU_LOCALDB_ERROR( _T( "트랜잭션 시작 실패 (DBName = {})" ),
                config_.name );
            return nullptr;
        }

        return std::make_unique<RocksDBTrans>( config_, check_rollback, trans_impl_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using ImplType = rocksdb::TransactionDB;

private:
    ImplType* impl_ { nullptr };
};

} // hu
