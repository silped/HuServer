export module hu.db.local.LocalDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LocalDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <filesystem>;
import <memory>;

import hu.db.local.impl.RocksDB;
import hu.db.local.LocalDBBase;
import hu.db.local.LocalDBConfig;
import hu.db.local.LocalDBType;
import hu.log.Log;
import hu.pattern.INoCopy;
import hu.serial.Serializer;
import hu.Type;
import hu.Util;

import "hu/db/local/LocalDBMacro.hpp";


namespace hu {

// 로컬 디비 트랜잭션을 구현한 클래스
export class LocalDBTransaction : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    LocalDBTransaction() = default;
    ~LocalDBTransaction() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 초기화 한다.
    bool Init(
        LocalDBTransPtr&& impl
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
        const LocalDBKey& key,
        const auto&       obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        buf.reserve( T::GetWriteSize( obj ) );
        if ( T::Write( obj, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, get_localdb_category_str(), _T( "객체 버퍼 쓰기 실패 (Key == {})" ), key );
            return false;
        }

        if ( impl_->Write( key, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, get_localdb_category_str(), _T( "디비 쓰기 실패 (Key = {})" ), key );
            return false;
        }

        return true;
    }

    // 디비에서 객체를 읽는다.
    template <typename T = Serializer>
    bool Read(
        const LocalDBKey& key,
        auto&             obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( impl_->Read( key, buf ) == false )
            return false;

        if ( T::Read( buf, obj ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, get_localdb_category_str(), _T( "객체 버퍼 읽기 실패 (Key == {})" ), key );
            return false;
        }

        return true;
    }

    // 디비에서 값을 삭제한다.
    bool Delete(
        const LocalDBKey& key,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( impl_->Delete( key ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, get_localdb_category_str(), _T( "디비 삭제 실패 (Key == {})" ), key );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (LocalDBTransaction)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    LocalDBTransPtr impl_;
};

// 로컬 디비를 구현한 클래스
export class LocalDB : private INoCopy
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
            HU_LOCALDB_ERROR( _T( "설정이 유효하지 않음 (DBName = {})" ),
                config.name );
            return false;
        }
        
        if ( config_.dir.empty() == false )
        {
            const String dir_path { util::get_format_str( _T( "{}/{}/" ), util::get_cur_path_str(), config.dir ) };
            if ( std::filesystem::exists( dir_path ) == false )
                std::filesystem::create_directory( dir_path );
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOCALDB_ERROR( _T( "구현체 생성 실패 (DBName = {}, Impl = {})" ),
                config.name, LocalDBImplTypeInfo::ToStr( config.impl ) );
            return false;
        }

        if ( impl_->Open() == false )
        {
            HU_LOCALDB_ERROR( _T( "디비 열기 실패 (DBName = {})" ),
                config.name );
            return false;
        }

        return true;
    }

    // 트랜잭션을 생성한다.
    bool CreateTransaction(
        LocalDBTransaction&        transaction,
        const LocalDBCheckRollback check_rollback = nullptr,
        const SrcLocation          loc = SrcLocation::current()
    ) const
    {
        if ( transaction.Init( impl_->CreateTrans( check_rollback ) ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, get_localdb_category_str(), _T( "트랜잭션 생성 실패 (DBName == {})" ), config_.name );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (LocalDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using ImplType = std::unique_ptr<LocalDBBase>;

private:
    static ImplType create_impl(
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
    ImplType          impl_;
};

} // hu
