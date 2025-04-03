export module hu.test.impl.TestSerial;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestSerial)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.serial.Serializer;
import hu.test.TestType;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestSerial)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_serial()
{
    // 직렬화 가능 객체는 개별 지정 초기화가 가능하다.
    SerialInfo src
    {
        .id = 100,
        .data = _T( "데이터" ),
        .member = _T( "멤버" )
    };

    // 바이너리 객체 크기에 맞춘 버퍼를 준비한다.
    const Size buf_size = BinSerializer::GetWriteSize( src );
    Buffer buf;
    buf.reserve( buf_size );

    // 객체를 바이너리로 버퍼에 쓴다.
    HU_ASSERT( BinSerializer::Write( src, buf ) );
    HU_ASSERT( buf.size() == buf_size );
    HU_ASSERT( buf.capacity() == buf_size );

    // 바이너리로 기록된 버퍼에서 객체를 읽는다.
    SerialInfo target;
    HU_ASSERT( BinSerializer::Read( buf, target ) );
    HU_ASSERT( src == target );
    target.Test();

    // JSON 객체 크기에 맞춘 버퍼를 준비한다.
    const Size json_buf_size = JsonSerializer::GetWriteSize( src );
    Buffer json_buf;
    json_buf.reserve( json_buf_size );

    // 객체를 JSON으로 버퍼에 쓴다.
    HU_ASSERT( JsonSerializer::Write( src, json_buf ) );
    HU_ASSERT( json_buf.size() == json_buf_size );
    HU_ASSERT( json_buf.capacity() == json_buf_size );

    // JSON으로 기록된 버퍼에서 객체를 읽는다.
    target.Clear();
    HU_ASSERT( JsonSerializer::Read( json_buf, target ) );
    HU_ASSERT( src == target );
    target.Test();

    return true;
}
