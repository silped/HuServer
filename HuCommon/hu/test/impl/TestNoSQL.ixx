export module hu.test.impl.TestNoSQL;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestNoSQLDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.test.TestType;

import "hu/db/nosql/NoSQL.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestNoSQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_nosql()
{
    // 디비 연결을 설정한다.
    NoSQLConfigInfo conf;
    {
        conf.db       = _T( "Game" );
        conf.user     = _T( "hu" );
        conf.password = _T( "pRaZTD3dR8wUUnS5" );
        conf.host     = _T( "cluster0.tbcuy4j.mongodb.net" );
        conf.impl     = ENoSQLImpl::kMongoDB;

        HU_ASSERT_R( conf.AddOption( _T( "retryWrites" ), _T( "true" ) ) );
        HU_ASSERT_R( conf.AddOption( _T( "w" ), _T( "majority" ) ) );
        HU_ASSERT_R( conf.AddOption( _T( "appName" ), _T( "Cluster0" ) ) );
    }

    // 설정을 검사한다.
    HU_ASSERT_R( conf.IsValid() );
    HU_LOG_NDEBUG( _T( "URI = {}" ), conf.GetUri() );

    // 디비에 연결한다.
    NoSQL db;
    HU_ASSERT_R( db.Connect( conf ) );

    const auto uuid1 = util::generate_uuid();

    // 객체를 쓴다.
    {
        SerialInfo obj;
        HU_ASSERT_R( db.Read( uuid1, obj ) == ENoSQLResult::kNotFound );

        obj.id          = 1;
        obj.data        = util::to_str( uuid1 );
        obj.member.data = _T( "멤버" );

        HU_ASSERT_R( db.Write( uuid1, obj ) );
    }

    // 객체를 읽는다.
    {
        SerialInfo obj;
        HU_ASSERT_R( db.Read( uuid1, obj ) == ENoSQLResult::kSuccess );
        obj.Test();

        HU_ASSERT_R( obj.id == 1 );
        HU_ASSERT_R( obj.data == util::to_str( uuid1 ) );
        HU_ASSERT_R( obj.member.data == _T( "멤버" ) );
    }

    const auto uuid2 = util::generate_uuid();

    // 객체를 쓴다.
    {
        SerialInfo obj;
        obj.id          = 2;
        obj.data        = util::to_str( uuid2 );
        obj.member.data = _T( "멤버2" );

        HU_ASSERT_R( db.Write( uuid2, obj ) );
    }

    // 객체 목록을 읽는다.
    {
        std::map<NoSQLId, SerialInfo> obj_map;
        HU_ASSERT_R( db.ReadList( { uuid1, uuid2 }, obj_map ) == 2 );

        for ( const auto& [ id, obj ] : obj_map )
            obj.Test();
    }

    // 데이터를 지운다.
    {
        HU_ASSERT_R( db.Delete<SerialInfo>( uuid1 ) == ENoSQLResult::kSuccess );
        HU_ASSERT_R( db.Delete<SerialInfo>( uuid2 ) == ENoSQLResult::kSuccess );

        SerialInfo obj;
        HU_ASSERT_R( db.Read( uuid1, obj ) == ENoSQLResult::kNotFound );
        HU_ASSERT_R( db.Read( uuid2, obj ) == ENoSQLResult::kNotFound );
    }

    return true;
}
