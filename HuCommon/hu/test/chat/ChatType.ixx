export module hu.test.chat.ChatType;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ChatType)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.container.MapBase;
import hu.net.NetType;
import hu.Type;

import "hu/serial/SerialMacro.hpp";


using namespace hu;

enum PacketType
{
    kPacketGenerateSeed = 100,
    kLoginReq,
    kLoginRes,
    kChatNoti,
};

// 로그인 요청
export struct LoginReq
{
    String user_name; // 유저 이름
    Buffer auth_key; // 인증키

    HU_SERIAL_MEMBER( LoginReq, user_name, auth_key );
};

// 로그인 응답
export struct LoginRes
{
    Buffer    auth_key; // 인증키
    ConnectId conn_id { 0 }; // 연결 아이디

    HU_SERIAL_MEMBER( LoginRes, auth_key, conn_id );
};

// 채팅 알림
export struct ChatNoti
{
    String user_name; // 유저 이름
    String data; // 메시지

    HU_SERIAL_MEMBER( ChatNoti, user_name, data );
};

// 유저 정보
export struct User
{
    String name; // 유저 이름
    bool   login { false }; // 로그인 여부

    void Clear()
    {
        name.clear();
        login = false;
    }
};

export using UserMap = MapBase<ConnectId, User>;
