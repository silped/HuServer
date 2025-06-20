export module hu.db.nosql.NoSQLDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (NoSQLDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <map>;

import hu.db.nosql.impl.MongoDB;
import hu.db.nosql.NoSQLDBType;

import "hu/Core.hpp";


namespace hu {

// NoSQL 디비를 구현한 클래스
export class NoSQLDB final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (NoSQLDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    NoSQLDB() = default;
    ~NoSQLDB() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (NoSQLDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 디비에 연결한다.
    bool Connect(
        const NoSQLDBConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "설정이 유효하지 않음 ({})" ), config.ToStr() );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "구현체 생성 실패 ({})" ), config_.ToStr() );
            return false;
        }

        if ( impl_->Connect() == false )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "디비 연결 실패 ({})" ), config_.ToStr() );
            return false;
        }

        return true;
    }

    // 디비에 객체를 쓴다.
    template <SerialType T>
    bool Write(
        const NoSQLDBId&  id,
        const T&          obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( JsonSerializer::Write( obj, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kNoSQLDB, _T( "객체 버퍼 쓰기 실패 (Table = {}, Id == {})" ),
                T::kTypeName, to_str( id ) );
            return false;
        }

        if ( impl_->Write( T::kTypeNameA, id, buf ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kNoSQLDB, _T( "디비 쓰기 실패 (Table = {}, Id = {})" ),
                T::kTypeName, to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 객체를 읽는다.
    template <SerialType T>
    bool Read(
        const NoSQLDBId&  id,
        T&                obj,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( impl_->Read( T::kTypeNameA, id, buf ) == false )
            return false;

        if ( JsonSerializer::Read( buf, obj ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kNoSQLDB, _T( "객체 버퍼 읽기 실패 (Table = {}, Id == {})" ),
                T::kTypeName, to_str( id ) );
            return false;
        }

        return true;
    }

    // 디비에서 객체 목록을 읽는다.
    template <SerialType T>
    Size ReadList(
        const NoSQLDBIdSet&     id_set,
        std::map<NoSQLDBId, T>& obj_map,
        const SrcLocation       loc = SrcLocation::current()
    ) const
    {
        NoSQLReadListResult res;
        if ( impl_->ReadList( T::kTypeNameA, id_set, res ) <= 0 )
            return 0;

        for ( const auto& [ id, buf ] : res )
        {
            T obj;
            if ( JsonSerializer::Read( buf, obj ) == false )
            {
                Log::Inst().Write( loc, LogType::kError, kNoSQLDB, _T( "객체 버퍼 읽기 실패 (Table = {}, Id == {})" ),
                    T::kTypeName, to_str( id ) );
                return 0;
            }

            obj_map.emplace( id, std::move( obj ) );
        }

        return obj_map.size();
    }

    // 디비에서 객체를 삭제한다.
    template <SerialType T>
    bool Delete(
        const NoSQLDBId&  id,
        const SrcLocation loc = SrcLocation::current()
    ) const
    {
        if ( impl_->Delete( T::kTypeNameA, id ) == false )
        {
            Log::Inst().Write( loc, LogType::kError, kNoSQLDB, _T( "디비 삭제 실패 (Table = {}, Id == {})" ),
                T::kTypeName, to_str( id ) );
            return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (NoSQLDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static NoSQLDBImpl create_impl(
        const NoSQLDBConfigInfo& config
    )
    {
        switch ( config.impl )
        {
        case NoSQLDBImplType::kMongoDB:
            return std::make_unique<MongoDB>( config );
        }

        return nullptr;
    }

private:
    NoSQLDBConfigInfo config_;
    NoSQLDBImpl       impl_;
};

} // hu
