export module hu.net.ServerConfig;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ServerConfig)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.ConnectionConfig;
import hu.net.NetType;
import hu.Type;


namespace hu {

// 서버 환경 설정
export struct ServerConfigInfo final
{
    // 서버 식별자
    ConnectId id { 1 };

    // 인터넷 프로토콜
    InternetProtocolType internet_protocol { InternetProtocolType::kV4 };

    // 포트
    UInt16 port { 10001 };

    // 전용 스레드로 실행 여부
    bool run_thread { false };

    // 사용할 네트워크 구현
    NetImplType net_impl { NetImplType::kAsio };

    // 최대 연결 개수
    UInt16 max_connection_count { 5000 };

    // 주소 재사용 여부
    bool reuse_address { true };

    // 연결 설정
    ConnectionConfigInfo connection;

    // 설정이 유효한지 검사한다.
    bool IsValid() const
    {
        if ( id <= 0 )
            return false;

        if ( port <= 0 )
            return false;

        if ( max_connection_count <= 0 )
            return false;

        if ( connection.IsValid() == false )
            return false;

        return true;
    }
};

} // hu
