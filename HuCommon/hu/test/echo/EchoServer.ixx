export module hu.test.echo.EchoServer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (EchoServer)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.io.Console;
import hu.log.Log;
import hu.net.Server;
import hu.test.echo.EchoType;
import hu.Type;
import hu.Util;

import "hu/log/LogMacro.hpp";


using namespace hu;

// 에코 서버를 구현한 클래스
class EchoServer
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (EchoServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    EchoServer() = default;
    ~EchoServer() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (EchoServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    inline static const String kCategory { _T( "EchoServer" ) };

    // 서버를 시작한다.
    void Start()
    {
        Console::Inst().SetTitle( kCategory );

        // 서버를 설정한다.
        ServerConfigInfo conf;
        {
            // 연결 시작 처리기를 설정한다.
            conf.connection.open_handler = [ this ]( const ConnectId conn_id )
            {
                HU_LOG_INFO( kCategory, _T( "클라이언트 연결 성공 (ConnId = {}, ConnCount = {}, ThreadId = {})" ),
                    conn_id, net_.GetConnectionCount(), util::get_thread_id() );
            };

            // 메시지 처리기를 설정한다.
            conf.connection.AddMessageHandler( [ this ]( const ConnectId conn_id, const Message& msg )
            {
                EchoMsg recv;
                if ( msg.ReadObj( recv ) )
                {
                    HU_LOG_INFO( kCategory, _T( "메시지 받음 (ConnId = {}, Data = {}, ThreadId = {})" ),
                        conn_id, recv.data, util::get_thread_id() );

                    // 서버에서 할당한 연결 아이디를 설정한다.
                    if ( recv.conn_id != conn_id )
                    {
                        recv.conn_id = conn_id;
                        HU_LOG_INFO( kCategory, _T( "연결 아이디 설정 (ConnId = {})" ), conn_id );
                    }
                }
                else
                {
                    HU_LOG_ERROR( kCategory, _T( "메시지 읽기 실패 (ConnId = {})" ), conn_id );
                }

                // 받은 메시지를 다시 보낸다.
                net_.SendObj( conn_id, recv );
                return true;
            } );

            // 연결 종료 처리기를 설정한다.
            conf.connection.close_handler = [ this ]( const ConnectId conn_id, const ConnectionCloseType close_type )
            {
                HU_LOG_INFO( kCategory, _T( "클라이언트 연결 종료 (ConnId = {}, CloseType = {}, ConnCount = {}, ThreadId = {})" ),
                    conn_id, ConnectionCloseTypeInfo::ToStr( close_type ), net_.GetConnectionCount(), util::get_thread_id() );
            };
        }

        // 네트워크 처리를 시작한다.
        net_.Start( conf );
    }

private:
    Server net_;
};

// 서버를 시작한다.
export void start_server()
{
    // 로그를 실행한다.
    LogConfigInfo log_conf;
    {
        log_conf.file.name = EchoServer::kCategory;
    }
    const LogRunner log_runner { log_conf };

    // 인스턴스를 시작한다.
    EchoServer server;
    server.Start();
}
