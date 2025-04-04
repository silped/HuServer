export module hu.net.ConnectionConfig;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ConnectionConfig)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <functional>;
import <unordered_map>;

import hu.crypt.CryptType;
import hu.net.Message;
import hu.net.NetType;
import hu.Type;


namespace hu {

// 연결 설정
export struct ConnectionConfigInfo final
{
    using OpenHandler       = std::function<void( const ConnectId )>;
    using ReceiveHandler    = std::function<bool( const ConnectId, const Message& )>;
    using CloseHandler      = std::function<void( const ConnectId, const ConnectionCloseType )>;
    using ReceiveHandlerMap = std::unordered_map<MessageId, ReceiveHandler>;

    // 헤더 종류
    HeaderType header { HeaderType::kSize2 };

    // 최대 메시지 크기
    Size max_msg_size { 1024 * 1024 };

    // 연결 시작 처리기
    OpenHandler open_handler;

    // 받은 메시지 처리기 맵
    ReceiveHandlerMap recv_handler_map;

    // 연결 종료 처리기
    CloseHandler close_handler;

    // 암호화 설정
    CryptConfigInfo crypt;

    // 소켓 보내기 버퍼 크기
    Int32 send_buffer_size { 1024 * 64 };

    // 소켓 받기 버퍼 크기
    Int32 recv_buffer_size { 1024 * 64 };

    // 소켓 닫힐때 남은 데이터를 보내기 위해 대기하는 시간 (초)
    Int32 linger_sec { 3 };

    // Nagle 알고리즘을 끄고 지연없이 즉시 전송 여부
    bool use_no_delay { true };

    // 소켓 연결 유지 확인 여부
    bool use_keep_alive { true };

    // 설정이 유효한지 검사한다.
    bool IsValid() const
    {
        if ( max_msg_size <= 0 )
            return false;

        if ( recv_handler_map.empty() )
            return false;

        return true;
    }

    // 메시지 처리기를 추가한다.
    bool AddMessageHandler(
        const ReceiveHandler& handler,
        const MessageId       msg_id = 0
    )
    {
        if ( recv_handler_map.contains( msg_id ) )
            return false;

        recv_handler_map[ msg_id ] = handler;
        return true;
    }

    // 메시지 처리기를 호출한다.
    bool HandleMessage(
        const ConnectId conn_id,
        const Message&  msg
    ) const
    {
        auto it = recv_handler_map.find( msg.GetId() );
        if ( it == recv_handler_map.cend() )
            it = recv_handler_map.find( 0 );

        if ( it != recv_handler_map.cend() )
            return it->second( conn_id, msg );

        return false;
    }
};

} // hu
