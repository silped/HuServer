export module hu.test.impl.TestSerial;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestSerial)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.test.TestType;

import "hu/Core.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestSerial)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_serial()
{
    // 직렬화 가능 객체는 개별 지정 초기화가 가능하다.
    SerialInfo src
    {
        .id     = 100,
        .data   = _T( "데이터" ),
        .member = _T( "멤버" )
    };

    SerialInfo target;

    const String data_dir = _T( "_data" );

    // 바이너리 직렬화를 검사한다.
    {
        using MySerializer = BinSerializer;

        // 버퍼를 준비한다.
        Buffer buffer;

        // 객체를 버퍼에 쓴다.
        HU_ASSERT_R( MySerializer::Write( src, buffer ) );

        // 버퍼에서 객체를 읽는다.
        target.Clear();
        HU_ASSERT_R( MySerializer::Read( buffer, target ) );
        HU_ASSERT_R( src == target );
        target.Test();

        // 객체를 파일에 쓴다.
        HU_ASSERT_R( MySerializer::WriteToFile( src, data_dir ) );

        // 파일에서 객체를 읽는다.
        target.Clear();
        HU_ASSERT_R( MySerializer::ReadFromFile( target, data_dir ) );
        HU_ASSERT_R( src == target );
        target.Test();
    }

    // JSON 직렬화를 검사한다.
    {
        using MySerializer = JsonSerializer;

        // 버퍼를 준비한다.
        Buffer buffer;

        // 객체를 버퍼에 쓴다.
        HU_ASSERT_R( MySerializer::Write( src, buffer ) );

        // 버퍼에서 객체를 읽는다.
        target.Clear();
        HU_ASSERT_R( MySerializer::Read( buffer, target ) );
        HU_ASSERT_R( src == target );
        target.Test();

        // 객체를 파일에 쓴다.
        HU_ASSERT_R( MySerializer::WriteToFile( src, data_dir ) );

        // 파일에서 객체를 읽는다.
        target.Clear();
        HU_ASSERT_R( MySerializer::ReadFromFile( target, data_dir ) );
        HU_ASSERT_R( src == target );
        target.Test();
    }

    return true;
}
