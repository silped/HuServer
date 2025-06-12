export module hu.db.local.LocalDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LocalDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <filesystem>;

import hu.db.local.impl.RocksDB;
import hu.db.local.LocalDBType;

import "hu/Core.hpp";


namespace hu {

// 로컬 디비 트랜잭션을 구현한 클래스
export class LocalDBTrans final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    LocalDBTrans() = default;
    ~LocalDBTrans() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 초기화 한다.
    bool Init(
        LocalDBTransImpl&& impl
    )
    {
        if ( impl == nullptr )
            return false;

        impl_ = std::move( impl );
        return true;
    }

    // 디비에 객체를 쓴다.
    template <typename T = Serializer>
    bool Write(
        const LocalDBId&  id,
        const auto&       obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( T::Write( obj, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kLocalDB, _T( "객체 버퍼 쓰기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        if ( impl_->Write( id, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kLocalDB, _T( "디비 쓰기 실패 (Id = {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 객체를 읽는다.
    template <typename T = Serializer>
    bool Read(
        const LocalDBId&  id,
        auto&             obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( impl_->Read( id, buf ) == false )
            return false;

        if ( T::Read( buf, obj ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kLocalDB, _T( "객체 버퍼 읽기 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 값을 삭제한다.
    bool Delete(
        const LocalDBId&  id,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( impl_->Delete( id ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kLocalDB, _T( "디비 삭제 실패 (Id == {})" ),
                to_str( id ) );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    LocalDBTransImpl impl_;
};

// 로컬 디비를 구현한 클래스
export class LocalDB final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LocalDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    LocalDB() = default;
    ~LocalDB() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (LocalDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 디비를 열고 초기화한다.
    bool Open(
        const LocalDBConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_LOG_ERROR( kLocalDB, _T( "설정이 유효하지 않음 (Table = {})" ),
                config.table );
            return false;
        }
        
        if ( config_.dir.empty() == false )
        {
            const String dir_path { util::format_str( _T( "{}/{}/" ), util::get_cur_path_str(), config.dir ) };
            if ( std::filesystem::exists( dir_path ) == false )
                std::filesystem::create_directory( dir_path );
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kLocalDB, _T( "구현체 생성 실패 (Table = {}, Impl = {})" ),
                config.table, LocalDBImplTypeInfo::ToStr( config.impl ) );
            return false;
        }

        if ( impl_->Open() == false )
        {
            HU_LOG_ERROR( kLocalDB, _T( "테이블 열기 실패 (Table = {})" ),
                config.table );
            return false;
        }

        return true;
    }

    // 트랜잭션을 생성한다.
    bool CreateTrans(
        LocalDBTrans&              trans,
        const LocalDBCheckRollback check_rollback = nullptr,
        const SrcLocation          loc = SrcLocation::current()
    ) const
    {
        if ( trans.Init( impl_->CreateTrans( check_rollback ) ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kLocalDB, _T( "트랜잭션 생성 실패 (Table == {})" ),
                config_.table );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (LocalDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static LocalDBImpl create_impl(
        const LocalDBConfigInfo& config
    )
    {
        switch ( config.impl )
        {
        case LocalDBImplType::kRocksDB:
            return std::make_unique<RocksDB>( config );
        }

        return nullptr;
    }

private:
    LocalDBConfigInfo config_;
    LocalDBImpl       impl_;
};

} // hu
