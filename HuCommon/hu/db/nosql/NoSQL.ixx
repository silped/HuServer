export module hu.db.nosql.NoSQL;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (NoSQLDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <map>;

import hu.db.nosql.impl.MongoDB;
import hu.db.nosql.NoSQLType;

import "hu/Core.hpp";


namespace hu {

// NoSQL 디비를 구현한 클래스
export class NoSQL final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (NoSQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    NoSQL() = default;
    ~NoSQL() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (NoSQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 디비에 연결한다.
    bool Connect(
        const NoSQLConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_LOG_ERROR( kNoSQL, _T( "설정이 유효하지 않음 ({})" ),
                config.ToStr() );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
        {
            HU_LOG_ERROR( kNoSQL, _T( "구현체 생성 실패 ({})" ),
                config_.ToStr() );
            return false;
        }

        if ( impl_->Connect() == false )
        {
            HU_LOG_ERROR( kNoSQL, _T( "디비 연결 실패 ({})" ),
                config_.ToStr() );
            return false;
        }

        return true;
    }

    // 디비에 객체를 쓴다.
    template <SerialType T>
    bool Write(
        const NoSQLId&     id,
        const T&           obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        if ( JsonSerializer::Write( obj, buf, loc ) )
            return impl_->Write( T::kTypeNameA, id, buf, loc );

        return false;
    }

    // 디비에서 객체를 읽는다.
    template <SerialType T>
    ENoSQLResult Read(
        const NoSQLId&     id,
        T&                 obj,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        Buffer buf;
        const auto res = impl_->Read( T::kTypeNameA, id, buf, loc );
        if ( res == ENoSQLResult::kSuccess )
        {
            if ( JsonSerializer::Read( buf, obj, loc ) == false )
                return ENoSQLResult::kFail;
        }

        return res;
    }

    // 디비에서 객체 목록을 읽는다.
    template <SerialType T>
    Size ReadList(
        const NoSQLIdSet&     id_set,
        std::map<NoSQLId, T>& obj_map,
        const SrcLocation&    loc = SrcLocation::current()
    ) const
    {
        NoSQLReadListResult res;
        if ( impl_->ReadList( T::kTypeNameA, id_set, res, loc ) > 0 )
        {
            for ( const auto& [ id, buf ] : res )
            {
                T obj;
                if ( JsonSerializer::Read( buf, obj, loc ) == false )
                    return 0;

                obj_map.emplace( id, std::move( obj ) );
            }

            return obj_map.size();
        }

        return 0;
    }

    // 디비에서 객체를 삭제한다.
    template <SerialType T>
    ENoSQLResult Delete(
        const NoSQLId&     id,
        const SrcLocation& loc = SrcLocation::current()
    ) const
    {
        return impl_->Delete( T::kTypeNameA, id, loc );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (NoSQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static NoSQLImpl create_impl(
        const NoSQLConfigInfo& config
    )
    {
        switch ( config.impl )
        {
        case ENoSQLImpl::kMongoDB:
            return std::make_unique<MongoDB>( config );
        }

        return nullptr;
    }

private:
    NoSQLConfigInfo config_;
    NoSQLImpl       impl_;
};

} // hu
