export module hu.net.ClientConfig;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ClientConfig)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.ConnectionConfig;
import hu.net.NetType;
import hu.Type;

import "hu/Macro.hpp";


namespace hu {

// 클라이언트 환경 설정
export struct ClientConfigInfo final
{
    // 클라이언트 식별자
    ConnectId id { 1 };

    // 연결 주소
    String address { _T( "localhost" ) };

    // 연결 포트
    UInt16 port { 10001 };

    // 전용 스레드로 실행 여부
    bool run_thread { false };

    // 사용할 네트워크 구현
    NetImplType net_impl { NetImplType::kAsio };

    // 연결 설정
    ConnectionConfigInfo connection;

    // 설정이 유효한지 검사한다.
    bool IsValid() const
    {
        if ( address.empty() )
            return false;

        if ( port <= 0 )
            return false;

        if ( connection.IsValid() == false )
            return false;

        return true;
    }
};

} // hu
