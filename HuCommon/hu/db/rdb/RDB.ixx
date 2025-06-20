export module hu.db.rdb.RDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (RDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.rdb.impl.MySQL;
import hu.db.rdb.RDBType;

import "hu/Core.hpp";


namespace hu {

// 트랜잭션을 구현한 클래스
export class RDBTrans final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    RDBTrans() = default;
    ~RDBTrans() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RDBTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 초기화 한다.
    bool Init(
        RDBTransImpl&& impl
    )
    {
        if ( impl == nullptr )
            return false;

        impl_ = std::move( impl );
        return true;
    }

    // 디비에 객체를 쓴다.
    template <SerialType T>
    bool Write(
        const RDBId&      id,
        const T&          obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( JsonSerializer::Write( obj, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kRDB, _T( "객체 버퍼 쓰기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        if ( impl_->Write( T::kTypeNameA, id, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kRDB, _T( "디비 쓰기 실패 (Id = {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 객체를 읽는다.
    template <SerialType T>
    bool Read(
        const RDBId&      id,
        T&                obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( impl_->Read( T::kTypeNameA, id, buf ) == false )
            return false;

        if ( JsonSerializer::Read( buf, obj ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kRDB, _T( "객체 버퍼 읽기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 삭제한다.
    template <SerialType T>
    bool Delete(
        const RDBId&      id,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( impl_->Delete( T::kTypeNameA, id ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kRDB, _T( "디비 삭제 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    RDBTransImpl impl_;
};

// 디비를 구현한 클래스
export class RDB final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (RDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    RDB() = default;
    ~RDB() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (RDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 디비에 연결한다.
    bool Connect(
        const RDBConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_LOG_ERROR( kRDB, _T( "설정이 유효하지 않음 ({})" ), config.ToStr() );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kRDB, _T( "구현체 생성 실패 ({})" ), config.ToStr() );
            return false;
        }

        if ( impl_->Connect() == false )
        {
            HU_LOG_ERROR( kRDB, _T( "디비 연결 실패 ({})" ), config.ToStr() );
            return false;
        }

        return true;
    }

    // 테이블을 생성한다.
    template <SerialType T>
    bool CreateTable()
    {
        return impl_->CreateTable( T::kTypeNameA );
    }

    // 트랜잭션을 생성한다.
    bool CreateTrans(
        RDBTrans&              trans,
        const RDBCheckRollback check_rollback = nullptr,
        const SrcLocation      loc = SrcLocation::current()
    ) const
    {
        if ( trans.Init( impl_->CreateTrans( check_rollback ) ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kRDB, _T( "트랜잭션 생성 실패 ({})" ), config_.ToStr() );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (RDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static RDBImpl create_impl(
        const RDBConfigInfo& config
    )
    {
        switch ( config.impl )
        {
        case RDBImplType::kMySQL:
            return std::make_unique<MySQL>( config );
        }

        return nullptr;
    }

private:
    RDBConfigInfo config_;
    RDBImpl       impl_;
};

} // hu
