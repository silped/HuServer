export module hu.test.echo.EchoType;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (EchoType)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.NetType;
import hu.Type;

import "hu/serial/SerialMacro.hpp";


using namespace hu;

// 통신에 사용할 객체는 직렬화를 선언한다.
export struct EchoMsg
{
    enum { kEchoMsg = 1 };

    ConnectId conn_id { 0 };
    String    data;

    HU_SERIAL_MEMBER( EchoMsg, conn_id, data );
};
