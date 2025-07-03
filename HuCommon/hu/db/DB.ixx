export module hu.db.DB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (DB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.rdb.impl.MySQL;
import hu.db.local.impl.RocksDB;
import hu.db.DBType;

import "hu/Core.hpp";


namespace hu {

// 트랜잭션을 구현한 클래스
export template <typename T>
class DBTrans final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (DBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    DBTrans() = default;
    ~DBTrans() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (DBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 초기화 한다.
    bool Init(
        DBTransImpl&& impl
    )
    {
        if ( impl == nullptr )
            return false;

        impl_  = std::move( impl );
        debug_ = impl_->IsDebug();

        return true;
    }

    // 디비에 객체를 쓴다.
    template <SerialType ST>
    EDBResult Write(
        const DBId&        id,
        const ST&          obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( T::Write( obj, buf, loc ) == false )
            return EDBResult::kFailToSerial;

        const auto res = impl_->Write( obj.kTypeNameA, id, buf, loc );
        if ( res == EDBResult::kSuccess )
        {
            if ( debug_ )
                util::log_debug( obj.kTypeName, util::to_str( obj ), loc );
        }

        return res;
    }

    // 디비에 객체를 쓴다.
    template <SerialType ST>
    EDBResult Write(
        const ST&          obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        return Write( obj.id, obj, loc );
    }

    // 디비에서 객체를 읽는다.
    template <SerialType ST>
    EDBResult Read(
        const DBId&        id,
        ST&                obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        const auto res = impl_->Read( obj.kTypeNameA, id, buf, loc );
        if ( res == EDBResult::kSuccess )
        {
            if ( T::Read( buf, obj, loc ) == false )
                return EDBResult::kFailToSerial;

            if ( debug_ )
                util::log_debug( obj.kTypeName, util::to_str( obj ), loc );
        }

        return res;
    }

    // 디비에서 객체를 읽는다.
    template <SerialType ST>
    EDBResult Read(
        ST&                obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        return Read( obj.id, obj, loc );
    }

    // 디비에서 삭제한다.
    template <SerialType ST>
    EDBResult Delete(
        const DBId&        id,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        return impl_->Delete( ST::kTypeNameA, id, loc );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (DBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    DBTransImpl impl_;
    bool        debug_ { false };
};

// 디비를 구현한 클래스
export template <typename T, EDBImpl kImpl>
class DB final : private INoCopy
{
public:
    using Trans = DBTrans<T>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (DB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    DB() = default;
    ~DB() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (DB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 디비에 연결한다.
    bool Connect(
        const DBConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_LOG_ERROR( kDB, _T( "설정이 유효하지 않음 ({})" ),
                config.ToStr() );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kDB, _T( "구현체 생성 실패 ({})" ),
                config.ToStr() );
            return false;
        }

        if ( impl_->Connect() == false )
        {
            HU_LOG_ERROR( kDB, _T( "디비 연결 실패 ({})" ),
                config.ToStr() );
            return false;
        }

        return true;
    }

    // 트랜잭션을 생성한다.
    bool CreateTrans(
        Trans&             trans,
        const DBRollback   rollback = nullptr,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        if ( config_.create_trans == false )
        {
            if ( trans.Init( impl_->CreateTrans( rollback, loc ) ) )
                return true;
        }

        util::log_error( loc, kDB, _T( "트랜잭션 생성 실패 ({})" ),
            config_.ToStr() );
        return false;
    }

    // 단일 트랜잭션으로 디비에 객체를 쓴다.
    template <SerialType ST>
    EDBResult WriteOne(
        const DBId&        id,
        const ST&          obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        Trans trans;
        if ( CreateTrans( trans, nullptr, loc ) == false )
            return EDBResult::kFailToTrans;

        return trans.Write( id, obj, loc );
    }

    // 단일 트랜잭션으로 디비에 객체를 쓴다.
    template <SerialType ST>
    EDBResult WriteOne(
        const ST&          obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        return WriteOne( obj.id, obj, loc );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (DB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static DBImpl create_impl(
        DBConfigInfo& config
    )
    {
        if ( config.impl == EDBImpl::kNone )
            config.impl = kImpl;

        switch ( config.impl )
        {
        case EDBImpl::kRocksDB:
            return std::make_unique<RocksDB>( config );
        case EDBImpl::kMySQL:
            return std::make_unique<MySQL>( config );
        }

        return nullptr;
    }

private:
    DBConfigInfo config_;
    DBImpl       impl_;
};

export using LDB = DB<BinSerializer, EDBImpl::kRocksDB>;
export using RDB = DB<JsonSerializer, EDBImpl::kMySQL>;

} // hu
