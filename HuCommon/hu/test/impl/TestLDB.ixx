export module hu.test.impl.TestLDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestLDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.test.TestType;

import "hu/db/DB.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestLDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_ldb()
{
    using MyDB      = LDB;
    using MyDBTrans = MyDB::Trans;

    // 디비를 선언한다.
    MyDB db;

    // 디비를 설정하고 연결한다.
    {
        DBConfigInfo conf;
        {
            // 트랜잭션 디버깅 활성화
            conf.debug_trans = true;

            // 디비 이름 설정
            conf.db  = _T( "hu_db" );

            // 디비 저장 디렉터리 설정
            conf.dir = _T( "_db" );

            // 테이블 정보 설정
            conf.AddTable<SerialInfo>();
        }
        HU_ASSERT_R( db.Connect( conf ) );
    }

    const auto uuid1 = util::generate_uuid();

    // 트랜잭션을 생성해서 객체를 디비에 쓴다. 성공하면 트랜잭션 범위에서 벗어날때 자동으로 커밋되고 실패하면 롤백된다.
    {
        MyDBTrans trans;
        HU_ASSERT_R( db.CreateTrans( trans ) );

        // 트랜잭션 중복해서 생성하면 실패한다.
        MyDBTrans trans2;
        HU_ASSERT_R( db.CreateTrans( trans2 ) == false );

        SerialInfo objr;
        HU_ASSERT_R( trans.Read( uuid1, objr ) == EDBResult::kNotFound );

        SerialInfo objw { 1, _T( "데이터" ), _T( "멤버" ) };
        HU_ASSERT_R( is_success( trans.Write( uuid1, objw ) ) );
    }

    // 디비에서 객체를 읽는다.
    {
        MyDBTrans trans;
        HU_ASSERT_R( db.CreateTrans( trans ) );

        SerialInfo obj;
        HU_ASSERT_R( is_success( trans.Read( uuid1, obj ) ) );
        obj.Test();
    }

    // 디비에서 객체를 삭제한다. 성공하면 트랜잭션 범위에서 벗어날때 자동으로 커밋되고 실패하면 롤백된다.
    {
        MyDBTrans trans;
        HU_ASSERT_R( db.CreateTrans( trans ) );
        HU_ASSERT_R( is_success( trans.Delete<SerialInfo>( uuid1 ) ) );
    }

    const auto uuid2 = util::generate_uuid();

    // 디비 작업은 성공 했으나 사용자 로직 검사에 실패해서 디비 작업을 롤백 시킨다.
    {
        bool check_logic = true;

        // 롤백 검사 함수
        const auto check_rollback = [ &check_logic ]()
        {
            return ( check_logic == false );
        };

        MyDBTrans trans;
        HU_ASSERT_R( db.CreateTrans( trans, check_rollback ) );

        SerialInfo objr;
        HU_ASSERT_R( trans.Read( uuid2, objr ) == EDBResult::kNotFound );

        SerialInfo objw { 2, _T( "데이터2" ), _T( "멤버2" ) };
        HU_ASSERT_R( is_success( trans.Write( uuid2, objw ) ) );

        // 로직 검사를 실패로 설정한다.
        check_logic = false;
    }

    // 롤백이 성공해서 데이터가 없는지 검사한다.
    {
        MyDBTrans trans;
        HU_ASSERT_R( db.CreateTrans( trans ) );

        SerialInfo obj;
        HU_ASSERT_R( trans.Read( uuid2, obj ) == EDBResult::kNotFound );
    }

    return true;
}
