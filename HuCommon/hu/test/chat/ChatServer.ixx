export module hu.test.chat.ChatServer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ChatServer)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.io.Console;
import hu.log.Log;
import hu.net.Message;
import hu.net.NetType;
import hu.net.Server;
import hu.test.chat.ChatType;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

// 채팅 서버를 구현한 클래스
class ChatServer final
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ChatServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    ChatServer() = default;
    ~ChatServer() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ChatServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    inline static const String kTitle { _T( "ChatServer" ) };

public:
    // 서버를 시작한다.
    void Start()
    {
        // 콘솔창 제목을 설정한다.
        Console::Inst().SetTitle( kTitle );

        // 서버를 설정한다.
        ServerConfigInfo conf;
        {
            ConnectionConfigInfo& conn_conf = conf.connection;

            // 여러 종류의 메시지를 처리할수 있게 식별자 포함 헤더를 사용한다.
            conn_conf.header = HeaderType::kSize2Id2;

            // 디피 헬먼 공개키 교환 방식 암호화를 사용한다.
            conn_conf.crypt.crypt_impl = CryptImplType::kBotanPk;

            // 연결 시작 처리기를 설정한다.
            conn_conf.open_handler = std::bind_front( &ChatServer::handle_open, this );

            // 연결 종료 처리기를 설정한다.
            conn_conf.close_handler = std::bind_front( &ChatServer::handle_close, this );

            // 메시지 처리기를 추가한다.
            conn_conf.AddMessageHandler( std::bind_front( &ChatServer::handle_login_rq, this ), LoginReq::kType );
            conn_conf.AddMessageHandler( std::bind_front( &ChatServer::handle_chat_noti, this ), ChatNoti::kType );
        }

        // 처리를 시작한다.
        net_.Start( conf );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (ChatServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    void handle_open( const ConnectId conn_id )
    {
        // 연결 성공하면 유저를 추가한다.
        if ( HU_IS_FAIL( user_map_.Add( conn_id, User() ) ) )
            return;

        HU_LOG_INFO( kTitle, _T( "클라이언트 연결 성공 (ConnId = {}, ConnCount = {}, UserCount = {})" ),
            conn_id, net_.GetConnectionCount(), user_map_.GetCount() );
    }

    void handle_close( const ConnectId conn_id, const ConnectionCloseType close_type )
    {
        // 연결 종료되면 유저를 제거한다.
        if ( HU_IS_FAIL( user_map_.Delete( conn_id ) ) )
            return;

        HU_LOG_INFO( kTitle, _T( "클라이언트 연결 종료 (ConnId = {}, CloseType = {}, ConnCount = {}, UserCount = {})" ),
            conn_id, ConnectionCloseTypeInfo::ToStr( close_type ), net_.GetConnectionCount(), user_map_.GetCount() );
    }

private:
    bool handle_login_rq( const ConnectId conn_id, const Message& msg )
    {
        // 메시지를 읽는다.
        LoginReq req;
        HU_ASSERT( msg.ReadObj( req ) );

        // 유저가 유효한지 검사한다.
        User* const user = user_map_.FindW( conn_id );
        HU_ASSERT( user != nullptr );
        HU_ASSERT( user->login == false );

        // 응답을 먼저 공개키를 설정해서 암호화 하지 않은 상태로 보낸다.
        LoginRes res;
        {
            res.auth_key = net_.GetCryptPublicKey( conn_id );
            res.conn_id  = conn_id;
        }
        HU_ASSERT( net_.SendObj( conn_id, res ) );

        // 요청의 공개키로 암호화 정보를 설정하면 이후 통신은 전부 암호화 된다.
        HU_ASSERT( net_.SetCrypt( conn_id, req.auth_key, conn_id ) );

        // 유저 정보를 설정한다.
        user->name  = req.user_name;
        user->login = true;

        HU_LOG_INFO( kTitle, _T( "로그인 성공 (ConId = {}, UserName = {}, ConnCount = {}, UserCount = {})" ),
            conn_id, user->name, net_.GetConnectionCount(), user_map_.GetCount() );
        return true;
    }

    bool handle_chat_noti( const ConnectId conn_id, const Message& msg )
    {
        // 메시지를 읽는다.
        ChatNoti noti;
        HU_ASSERT( msg.ReadObj( noti ) );

        // 유저가 유효한지 검사한다.
        const User* const user = user_map_.Find( conn_id );
        HU_ASSERT( user != nullptr );
        HU_ASSERT( user->login );
        HU_ASSERT( user->name == noti.user_name );

        // 모든 유저에게 보낸다.
        auto send_all = [ & ]( const ConnectId conn_id, const User& )
        {
            return net_.SendObj( conn_id, noti );
        };
        HU_ASSERT( user_map_.Loop( send_all ) );

        HU_LOG_INFO( kTitle, _T( "채팅 (ConnId = {}, UserName = {}, Data = {}, ConnCount = {}, UserCount = {})" ),
            conn_id, noti.user_name, noti.data, net_.GetConnectionCount(), user_map_.GetCount() );
        return true;
    }

private:
    Server  net_;
    UserMap user_map_;
};

// 서버를 시작한다.
export void start_server()
{
    // 로그를 실행한다.
    LogConfigInfo log_conf;
    {
        log_conf.file.name = ChatServer::kTitle;
    }
    const LogRunner log_runner { log_conf };

    // 인스턴스를 시작한다.
    ChatServer server;
    server.Start();
}
