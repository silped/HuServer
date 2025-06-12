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

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (RocksDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

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
        rocksdb::Transaction* const impl
    ) :
        LocalDBTransBase ( config, check_rollback ),
        impl_            ( impl )
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
                        config_.table, to_str( status ) );
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
        const LocalDBId& id,
        const Buffer&    buffer
    ) override
    {
        const auto status = impl_->Put( id, to_param( buffer ) );
        if ( status.ok() == false )
        {
            impl_->Rollback();
            util::delete_ptr( impl_ );

            HU_LOG_ERROR( kLocalDB, _T( "디비 쓰기 실패 (Table = {}, Id = {}, {})" ),
                config_.table, to_str( id ), to_str( status ) );
            return false;
        }

        return true;
    }

    virtual bool Read(
        const LocalDBId& id,
        Buffer&          buffer
    ) override
    {
        ParamType read_value;
        const auto status = impl_->Get( read_options_, id, &read_value );
        if ( status.ok() == false )
        {
            if ( status.IsNotFound() == false )
            {
                util::delete_ptr( impl_ );

                HU_LOG_ERROR( kLocalDB, _T( "디비 읽기 실패 (Table = {}, Id = {}, {})" ),
                    config_.table, to_str( id ), to_str( status ) );
            }

            return false;
        }

        buffer.assign( read_value.begin(), read_value.end() );
        return true;
    }

    virtual bool Delete(
        const LocalDBId& id
    ) override
    {
        const auto status = impl_->Delete( id );
        if ( status.ok() == false )
        {
            if ( status.IsNotFound() == false )
            {
                impl_->Rollback();
                util::delete_ptr( impl_ );

                HU_LOG_ERROR( kLocalDB, _T( "디비 삭제 실패 (Table = {}, Id = {}, {})" ),
                    config_.table, to_str( id ), to_str( status ) );
            }

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
    // 버퍼를 파라미터로 변환한다.
    static ParamType to_param(
        const Buffer& buffer
    )
    {
        return ParamType( buffer.cbegin(), buffer.cend() );
    }

private:
    rocksdb::Transaction* impl_ { nullptr };
    rocksdb::ReadOptions  read_options_;
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
        LocalDBBase( config )
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

        AString db_name;
        if ( config_.dir.empty() == false )
            db_name = util::to_utf8( util::format_str( _T( "{}/{}" ), config_.dir, config_.table ) );
        else
            db_name = util::to_utf8( config_.table );

        const auto status = rocksdb::TransactionDB::Open( options, trans_db_options, db_name, &impl_ );
        if ( ( status.ok() == false ) || ( impl_ == nullptr ) )
        {
            HU_LOG_ERROR( kLocalDB, _T( "디비 열기 실패 (Table = {}, {})" ), config_.table, to_str( status ) );
            return false;
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
            HU_LOG_ERROR( kLocalDB, _T( "트랜잭션 시작 실패 (Table = {})" ),
                config_.table );
            return nullptr;
        }

        return std::make_unique<RocksDBTrans>( config_, check_rollback, trans_impl_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RocksDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    rocksdb::TransactionDB* impl_ { nullptr };
};

} // hu
