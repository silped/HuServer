export module hu.db.nosql.impl.MongoDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (MongoDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.nosql.NoSQLDBType;

import <bsoncxx/json.hpp>;
import <mongocxx/client.hpp>;
import <mongocxx/instance.hpp>;

import "hu/Core.hpp";


namespace hu {

namespace Bson = bsoncxx::builder::basic;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (MongoDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

// MongoDB를 사용해서 구현한 로컬 디비 클래스
export class MongoDB final : public NoSQLDBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MongoDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit MongoDB(
        const NoSQLDBConfigInfo& config
    ) :
        NoSQLDBBase( config )
    {
    }

    ~MongoDB() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MongoDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Connect() override
    {
        const auto uri_str = config_.GetUri();

        try
        {
            const mongocxx::uri uri { to_param( uri_str ) };

            mongocxx::options::client options;
            {
                // API 호환성을 설정한다.
                const mongocxx::options::server_api api { mongocxx::options::server_api::version::k_version_1 };
                options.server_api_opts( api );
            }

            client_ = mongocxx::client { uri, options };
            db_     = client_[ to_param( config_.db ) ];

            // 쓸때 있으면 업데이트, 없으면 삽입하도록 설정한다.
            update_options_.upsert( true );

            return  true;
        }
        catch ( const std::exception& e )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "연결 실패 (Uri = {}, Error = {})" ),
                uri_str, util::utf8_to_str( e.what() ) );
        }

        return false;
    }

    virtual bool Write(
        const String&    table,
        const NoSQLDBId& id,
        const Buffer&    buffer
    ) override
    {
        if ( table.empty() || id.empty() || buffer.empty() )
            return false;

        try
        {
            auto coll = db_[ to_param( table ) ];

            const auto result = coll.update_one(
                Bson::make_document( Bson::kvp( kId, id ) ),
                Bson::make_document( Bson::kvp( kSet, bsoncxx::from_json( Param { buffer.cbegin(), buffer.cend() } ) ) ),
                update_options_ );
            if ( result && ( result->upserted_count() > 0 ) )
                return true;
        }
        catch ( const std::exception& e )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "쓰기 실패 (Table = {}, Id = {}, Error = {}" ),
                table, util::utf8_to_str( id ), util::utf8_to_str( e.what() ) );
        }

        return false;
    }

    virtual bool Read(
        const String&    table,
        const NoSQLDBId& id,
        Buffer&          buffer
    ) override
    {
        if ( table.empty() || id.empty() )
            return false;

        try
        {
            const auto table_param = to_param( table );
            if ( db_.has_collection( table_param ) == false )
                return false;

            auto coll = db_[ table_param ];

            const auto res = coll.find_one( Bson::make_document( Bson::kvp( kId, id ) ) );
            if ( res )
            {
                auto json = bsoncxx::to_json( *res );
                util::remove_space( json );
                buffer.assign( json.cbegin(), json.cend() );
                return true;
            }
        }
        catch ( const std::exception& e )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "읽기 실패 (Table = {}, Id = {}, Error = {})" ),
                table, util::utf8_to_str( id ), util::utf8_to_str( e.what() ) );
        }

        return false;
    }

    virtual Size ReadList(
        const String&        table,
        const NoSQLDBIdSet&  id_set,
        NoSQLReadListResult& result
    ) override
    {
        if ( table.empty() || id_set.empty() )
            return 0;

        try
        {
            const auto table_param = to_param( table );
            if ( db_.has_collection( table_param ) == false )
                return 0;

            Bson::array ids;
            for ( const auto& id : id_set )
                ids.append( id );

            auto coll = db_[ table_param ];

            auto cursor = coll.find( Bson::make_document( 
                Bson::kvp( kId, Bson::make_document( Bson::kvp( kIn, ids ) ) )
            ) );

            for ( const auto& doc : cursor )
            {
                auto json = bsoncxx::to_json( doc );
                util::remove_space( json );
                result.emplace( doc[ kId ].get_string().value, Buffer( json.cbegin(), json.cend() ) );
            }

            return result.size();
        }
        catch ( const std::exception& e )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "목록 읽기 실패 (Table = {}, Error = {})" ),
                table, util::utf8_to_str( e.what() ) );
        }

        return 0;
    }

    virtual bool Delete(
        const String&    table,
        const NoSQLDBId& id
    ) override
    {
        if ( table.empty() || id.empty() )
            return false;

        try
        {
            const auto table_param = to_param( table );
            if ( db_.has_collection( table_param ) == false )
                return false;

            auto coll = db_[ table_param ];

            auto res = coll.delete_one( Bson::make_document( Bson::kvp( kId, id ) ) );
            if ( res && ( res->deleted_count() > 0 ) )
                return true;
        }
        catch ( const std::exception& e )
        {
            HU_LOG_ERROR( kNoSQLDB, _T( "삭제 실패 (Table = {}, Id = {}, Error = {})" ),
                table, util::utf8_to_str( id ), util::utf8_to_str( e.what() ) );
        }

        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (MongoDB)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Param = AString;

private:
    static Param to_param(
        const String& str
    )
    {
        return util::to_utf8( str );
    }

private:
    inline static Param kId  { "_id" };
    inline static Param kSet { "$set" };
    inline static Param kIn  { "$in" };

    inline static mongocxx::instance instance_;

private:
    mongocxx::client          client_;
    mongocxx::database        db_;
    mongocxx::options::update update_options_;
};

} // hu
