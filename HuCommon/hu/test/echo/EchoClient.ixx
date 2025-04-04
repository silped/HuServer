export module hu.test.echo.EchoClient;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (EchoClient)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <atomic>;

import hu.container.LockFreeQueue;
import hu.io.Console;
import hu.log.Log;
import hu.net.Client;
import hu.test.echo.EchoType;
import hu.Type;
import hu.Util;

import "hu/Macro.hpp";


using namespace hu;

// 에코 클라이언트를 구현한 클래스
class EchoClient
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (EchoClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    EchoClient() :
        cs_ { Console::Inst() }
    {
    }

    ~EchoClient() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (EchoClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    inline static const String kCategory { _T( "EchoClient" ) };

    // 클라이언트를 시작한다.
    void Start()
    {
        cs_.SetTitle( kCategory );

        // 클라이언트를 설정한다.
        ClientConfigInfo conf;
        {
            // 네트워크를 로직 스레드와 별도 스레드로 처리한다.
            conf.run_thread = true;

            // 연결 시작 처리기를 설정한다.
            conf.connection.open_handler = [ this ]( const ConnectId conn_id )
            {
                cs_.WriteLn( _T( "서버 연결 성공 (ConnId = {}, ThreadId = {})" ),
                    conn_id, util::get_thread_id() );
            };

            // 메시지 처리기를 설정한다.
            conf.connection.AddMessageHandler( [ this ]( const ConnectId conn_id, const Message& msg )
            {
                EchoMsg recv;
                if ( msg.ReadObj( recv ) )
                {
                    // 서버에서 받은 연결 아이디를 설정한다.
                    if ( recv.conn_id != net_.GetConnectId() )
                    {
                        net_.SetConnectId( recv.conn_id );
                        cs_.WriteLn( _T( "연결 아이디 설정 (ConnId = {})" ), net_.GetConnectId() );
                    }

                    // 로직 스레드에서 처리할수 있게 받은 메시지 큐에 넣는다.
                    recv_queue_.push( recv );
                }
                else
                {
                    cs_.WriteLn( _T( "메시지 읽기 실패 (ConnId = {})" ), conn_id );
                }

                return true;
            } );

            // 연결 종료 처리기를 설정한다.
            conf.connection.close_handler = [ this ]( const ConnectId conn_id, const ConnectionCloseType close_type )
            {
                cs_.WriteLn( _T( "서버 접속 종료. 3초후 재연결 시도 (ConnId = {}, CloseType = {}, ThreadId = {})" ),
                    conn_id, ConnectionCloseTypeInfo::ToStr( close_type ), util::get_thread_id() );

                util::sleep_thread( 3000 );
                net_.Connect();
            };
        }

        // 네트워크 처리를 시작했을때 사용자 입력이 블럭되면 안되기 때문에 네트워크 전용 스레드에서 비동기로 시작한다.
        net_.Start( conf );

        // 메인 스레드에서 로직을 처리한다.
        while ( true )
        {
            // 서버 접속이 성공할때까지 대기한다.
            if ( net_.IsOpen() == false )
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
                net_.SendObj( EchoMsg { net_.GetConnectId(), input } );
            }

            // 서버에서 메시지를 받을때까지 대기한다.
            while ( net_.IsOpen() && recv_queue_.empty() )
            {
                cs_.WriteLn( _T( "." ) );
                util::sleep_thread( 1000 );
            }

            // 서버에서 받은 메시지를 처리한다.
            EchoMsg obj;
            if ( recv_queue_.pop( obj ) )
            {
                cs_.WriteLn( _T( "메시지 받음 (ConnId = {}, Data = {}, ThreadId = {})" ),
                    net_.GetConnectId(), obj.data, util::get_thread_id() );
            }
        }

        cs_.WriteLn( _T( "로직 처리 종료. 아무키나 누르세요. (ConnId = {}, ThreadId = {})" ),
            net_.GetConnectId(), util::get_thread_id() );
        cs_.Wait();
    }

private:
    Client   net_;
    Console& cs_;
    LockFreeSpscQueue<EchoMsg> recv_queue_;
};

// 클라이언트를 시작한다.
export void start_client()
{
    // 로그를 실행한다.
    LogConfigInfo log_conf;
    {
        log_conf.file.name = EchoClient::kCategory;
    }
    const LogRunner log_runner { log_conf };

    // 인스턴스를 시작한다.
    EchoClient client;
    client.Start();
}
