export module hu.test.chat.ChatClient;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ChatClient)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.io.Console;
import hu.log.Log;
import hu.net.Client;
import hu.net.Message;
import hu.net.NetType;
import hu.test.chat.ChatType;
import hu.Type;
import hu.Util;

import "hu/log/LogMacro.hpp";


using namespace hu;

// 채팅 클라이언트를 구현한 클래스
class ChatClient final
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ChatClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    ChatClient() :
        cs_ { Console::Inst() }
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ChatClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    inline static const String kTitle { _T( "ChatClient" ) };

public:
    // 클라이언트를 시작한다.
    void Start()
    {
        // 콘솔창 제목을 설정한다.
        cs_.SetTitle( kTitle );

        // 클라이언트를 설정한다.
        ClientConfigInfo conf;
        {
            // 사용자 입력 처리를 위해서 네트워크를 별도 스레드로 실행한다.
            conf.run_thread = true;

            ConnectionConfigInfo& conn_conf = conf.connection;

            // 여러 종류의 메시지를 처리할수 있게 식별자 포함 헤더를 사용한다.
            conn_conf.header = HeaderType::kSize2Id2;

            // 디피 헬먼 공개키 교환 방식 암호화를 사용한다.
            conn_conf.crypt.crypt_impl = CryptImplType::kBotanPk;

            // 연결 시작 처리기를 설정한다.
            conn_conf.open_handler = std::bind_front( &ChatClient::handle_open, this );

            // 연결 종료 처리기를 설정한다.
            conn_conf.close_handler = std::bind_front( &ChatClient::handle_close, this );

            // 메시지 처리기를 설정한다.
            conn_conf.AddMessageHandler( std::bind_front( &ChatClient::handle_login_res, this ), LoginRes::kType );
            conn_conf.AddMessageHandler( std::bind_front( &ChatClient::handle_chat_noti, this ), ChatNoti::kType );
        }

        // 로그인 정보를 입력한다.
        while ( user_name_.empty() )
        {
            cs_.Write( _T( "로그인 사용자 이름 입력 : " ) );
            user_name_ = cs_.ReadLn();
        }

        // 로그인 정보를 입력 받았으면 네트워크 처리를 시작한다.
        net_.Start( conf );

        // 채팅 입력을 메인 스레드에서 반복 처리한다.
        while ( true )
        {
            // 로그인이 성공할때까지 대기한다.
            if ( login_ == false )
            {
                cs_.WriteLn( _T( "." ) );
                util::sleep_thread( 1000 );
                continue;
            }

            // 사용자 입력을 처리한다.
            auto input = cs_.ReadLn();
            if ( input == _T( "q" ) )
            {
                net_.Stop();
                break;
            }
            else
            {
                HU_CHECK( net_.SendObj( ChatNoti { user_name_, input } ) );
            }
        }

        cs_.WriteLn( _T( "로직 처리 종료. 아무키나 누르세요. (ConnId = {}, ThreadId = {})" ),
            net_.GetConnectId(), util::get_thread_id() );
        cs_.Wait();
    }

private:
    // 연결이 성공하면 로그인 요청을 보낸다.
    void handle_open( const ConnectId conn_id )
    {
        cs_.WriteLn( _T( "서버 연결 성공 (ConnId = {}, ThreadId = {})" ),
            conn_id, util::get_thread_id() );

        LoginReq req;
        {
            req.user_name = user_name_;
            req.auth_key  = net_.GetCryptPublicKey();
        }
        HU_CHECK( net_.SendObj( req ) );
    }

    // 연결이 종료되면 로그아웃하고 재연결을 시도한다.
    void handle_close( const ConnectId conn_id, const ConnectionCloseType close_type )
    {
        login_ = false;

        cs_.WriteLn( _T( "서버 접속 종료. 1초후 재연결 시도 (ConnId = {}, CloseType = {}, ThreadId = {})" ),
            conn_id, ConnectionCloseTypeInfo::ToStr( close_type ), util::get_thread_id() );

        util::sleep_thread( 1000 );
        net_.Connect();
    }

    // 로그인 응답을 처리한다.
    bool handle_login_res( const ConnectId conn_id, const Message& msg )
    {
        LoginRes res;
        HU_ASSERT( msg.ReadObj( res ) );

        // 응답의 인증 정보를 설정한다.
        net_.SetConnectId( res.conn_id );
        net_.SetCrypt( res.auth_key, res.conn_id );

        cs_.WriteLn( _T( "로그인 성공 (OldConnId = {}, NewConnId = {}, ThreadId = {})" ),
            conn_id, res.conn_id, util::get_thread_id() );

        login_ = true;
        return true;
    }

    // 채팅 알림을 처리한다.
    bool handle_chat_noti( const ConnectId conn_id, const Message& msg )
    {
        ChatNoti noti;
        HU_ASSERT( msg.ReadObj( noti ) );

        cs_.WriteLn( _T( "채팅 ({} = {}, ConnId = {}, ThreadId = {})" ),
            noti.user_name, noti.data, conn_id, util::get_thread_id() );
        return true;
    }

private:
    Client            net_;
    Console&          cs_;
    String            user_name_;
    std::atomic<bool> login_ { false };
};

// 클라이언트를 시작한다.
export void start_client()
{
    // 로그를 실행한다.
    LogConfigInfo log_conf;
    {
        log_conf.file.name = ChatClient::kTitle;
    }
    const LogRunner log_runner { log_conf };

    // 인스턴스를 시작한다.
    ChatClient client;
    client.Start();
}
