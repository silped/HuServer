export module hu.net.impl.asio.AsioServer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AsioServer)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.net.impl.asio.AsioRunner;
import hu.net.impl.asio.AsioType;
import hu.net.impl.asio.AsioConnection;
import hu.net.ServerBase;
import hu.Util;

import "hu/net/NetMacro.hpp";


namespace hu {

// Asio를 사용해서 구현한 서버 클래스
export class AsioServer final : public ServerBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AsioServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit AsioServer(
        const ServerConfigInfo& config
    ) :
        ServerBase { config },
        runner_    { std::make_shared<AsioRunner>( config.run_thread ) },
        acceptor_  { runner_->GetContext() }
    {
    }

    ~AsioServer() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AsioServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Start() override
    {
        if ( ready_impl() == false )
        {
            HU_NET_ERROR( _T( "시작 준비 실패 (ServerId = {})" ),
                config_.id );
            return false;
        }

        if ( runner_->Run() == false )
        {
            HU_NET_ERROR( _T( "실행기 실행 실패 (ServerId = {})" ),
                config_.id );
            return false;
        }

        return true;
    }

    virtual void Stop() override
    {
        runner_->Stop();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (AsioServer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    bool ready_impl()
    {
        const AsioEndPoint ep { ( config_.internet_protocol == InternetProtocolType::kV6 ? AsioTcp::v6() : AsioTcp::v4() ), config_.port };

        AsioErrorCode ec;
        acceptor_.open( ep.protocol(), ec );
        if ( ec )
        {
            HU_NET_ERROR( _T( "프로토콜 열기 실패 (ServerId = {}, {})" ),
                config_.id, util::asio_error_str( ec ) );
            return false;
        }

        if ( config_.reuse_address )
        {
            acceptor_.set_option( AsioAcceptor::reuse_address( config_.reuse_address ), ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "ReuseAddress 옵션 설정 실패 (ServerId = {}, {})" ),
                    config_.id, util::asio_error_str( ec ) );
                return false;
            }
        }

        acceptor_.bind( ep, ec );
        if ( ec )
        {
            HU_NET_ERROR( _T( "Bind 실패(ServerId = {}, {})" ),
                config_.id, util::asio_error_str( ec ) );
            return false;
        }

        acceptor_.listen( AsioSocketBase::max_connections, ec );
        if ( ec )
        {
            HU_NET_ERROR( _T( "Listen 실패 (ServerId = {}, {})" ),
                config_.id, util::asio_error_str( ec ) );
            return false;
        }

        start_accept();

        HU_NET_INFO( _T( "서버 시작 준비 완료 (ServerId = {}, Port = {}, RunThread = {}, MaxConnCount = {}, Header = {}, MaxMsgSize = {}, Crypt = {}, ThreadId = {})" ),
            config_.id,
            config_.port,
            config_.run_thread,
            config_.max_connection_count,
            HeaderTypeInfo::ToStr( config_.connection.header ),
            config_.connection.max_msg_size,
            CryptImplTypeInfo::ToStr( config_.connection.crypt.crypt_impl ),
            util::get_thread_id() );
        return true;
    }

    void start_accept()
    {
        acceptor_.async_accept( [ this ]( const AsioErrorCode ec, AsioSocket socket )
        {
            if ( ec )
            {
                HU_NET_ERROR( _T( "Accept 실패 (ServerId = {}, {})" ),
                    config_.id, util::asio_error_str( ec ) );
            }
            else
            {
                if ( conn_manager_.IsMax() )
                {
                    AsioErrorCode ec2;
                    socket.close( ec2 );
                    if ( ec2 )
                    {
                        HU_NET_ERROR( _T( "소켓 닫기 실패 (ServerId = {}, {})" ),
                            config_.id, util::asio_error_str( ec ) );
                    }

                    HU_NET_ERROR( _T( "연결 개수 최대 (ServerId = {}, ConnCount = {})" ),
                        config_.id, conn_manager_.GetCount() );
                }
                else
                {
                    const auto new_conn { std::make_shared<AsioConnection>( config_.connection, runner_, std::move( socket ) ) };
                    if ( conn_manager_.Open( new_conn ) == false )
                    {
                        HU_NET_ERROR( _T( "연결 열기 실패 (ServerId = {})" ),
                            config_.id );
                    }
                }
            }

            start_accept();
        } );
    }

private:
    AsioRunnerPtr runner_;
    AsioAcceptor  acceptor_;
};

} // hu
