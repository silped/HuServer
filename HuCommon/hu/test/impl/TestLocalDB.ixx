export module hu.test.impl.TestLocalDB;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestLocalDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.local.LocalDB;
import hu.db.local.LocalDBConfig;
import hu.db.local.LocalDBType;
import hu.log.Log;
import hu.test.TestType;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestLocalDB)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_local_db()
{
    LocalDBConfigInfo conf;

    LocalDB db;
    HU_ASSERT( db.Open( conf ) );

    // 데이터를 쓴다. 성공하면 트랜잭션 선언 범위에서 벗어날때 자동으로 커밋되고 실패하면 자동으로 롤백된다.
    {
        LocalDBTransaction trans;
        HU_ASSERT( db.CreateTransaction( trans ) );

        SerialInfo objr;
        HU_ASSERT( trans.Read( _T( "1" ), objr ) == false );

        SerialInfo objw { 1, _T( "데이터" ), _T( "멤버" ) };
        HU_ASSERT( trans.Write( _T( "1" ), objw ) );
    }

    // 데이터를 읽고 다시 삭제한다. 성공하면 트랜잭션 선언 범위에서 벗어날때 자동으로 커밋되고 실패하면 자동으로 롤백된다.
    {
        LocalDBTransaction trans;
        HU_ASSERT( db.CreateTransaction( trans ) );

        SerialInfo obj;
        HU_ASSERT( trans.Read( _T( "1" ), obj ) );
        obj.Test();

        trans.Delete( _T( "1" ) );
    }

    // 디비 쓰기는 성공 했으나 로직 검사에 실패해서 자동으로 롤백 시킨다.
    {
        bool check_logic = true;

        // 롤백 검사 함수
        const auto check_rollback = [ &check_logic ]()
        {
            return ( check_logic == false );
        };

        LocalDBTransaction trans;
        HU_ASSERT( db.CreateTransaction( trans, check_rollback ) );

        SerialInfo objr;
        HU_ASSERT( trans.Read( _T( "2" ), objr ) == false );

        SerialInfo objw { 2, _T( "데이터2" ), _T( "멤버2" ) };
        HU_ASSERT( trans.Write( _T( "2" ), objw ) );

        // 로직 검사를 실패로 설정한다.
        check_logic = false;
    }

    // 롤백이 성공해서 데이터가 없는지 검사한다.
    {
        LocalDBTransaction trans;
        HU_ASSERT( db.CreateTransaction( trans ) );

        SerialInfo obj;
        HU_ASSERT( trans.Read( _T( "2" ), obj ) == false );
    }

    return true;
}
