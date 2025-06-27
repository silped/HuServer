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

        impl_ = std::move( impl );
        return true;
    }

    // 디비에 객체를 쓴다.
    template <SerialType OBJ_T>
    bool Write(
        const DBId&       id,
        const OBJ_T&      obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( T::Write( obj, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kDB, _T( "객체 버퍼 쓰기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        if ( impl_->Write( obj.kTypeNameA, id, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kDB, _T( "디비 쓰기 실패 (Id = {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 객체를 읽는다.
    template <SerialType OBJ_T>
    bool Read(
        const DBId&       id,
        OBJ_T&            obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( impl_->Read( obj.kTypeNameA, id, buf ) == false )
            return false;

        if ( T::Read( buf, obj ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kDB, _T( "객체 버퍼 읽기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 삭제한다.
    template <SerialType OBJ_T>
    bool Delete(
        const DBId&       id,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( impl_->Delete( OBJ_T::kTypeNameA, id ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kDB, _T( "디비 삭제 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (DBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    DBTransImpl impl_;
};

// 디비를 구현한 클래스
export template <typename T, DBImplType kImpl>
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
            HU_LOG_ERROR( kDB, _T( "설정이 유효하지 않음 ({})" ), config.ToStr() );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kDB, _T( "구현체 생성 실패 ({})" ), config.ToStr() );
            return false;
        }

        if ( impl_->Connect() == false )
        {
            HU_LOG_ERROR( kDB, _T( "디비 연결 실패 ({})" ), config.ToStr() );
            return false;
        }

        return true;
    }

    // 트랜잭션을 생성한다.
    bool CreateTrans(
        Trans&            trans,
        const DBRollback  rollback = nullptr,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( config_.create_trans == false )
        {
            if ( trans.Init( impl_->CreateTrans( rollback ) ) )
                return true;
        }

        Log::Inst().Write( loc, LogType::kError, kDB, _T( "트랜잭션 생성 실패 ({})" ),
            config_.ToStr() );
        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (DB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static DBImpl create_impl(
        DBConfigInfo& config
    )
    {
        if ( config.impl == DBImplType::kNone )
            config.impl = kImpl;

        switch ( config.impl )
        {
        case DBImplType::kRocksDB:
            return std::make_unique<RocksDB>( config );
        case DBImplType::kMySQL:
            return std::make_unique<MySQL>( config );
        }

        return nullptr;
    }

private:
    DBConfigInfo config_;
    DBImpl       impl_;
};

export using LDB = DB<BinSerializer, DBImplType::kRocksDB>;
export using RDB = DB<JsonSerializer, DBImplType::kMySQL>;

} // hu
