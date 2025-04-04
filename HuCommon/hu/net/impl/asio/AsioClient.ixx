export module hu.net.impl.asio.AsioClient;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AsioClient)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.net.ClientBase;
import hu.net.impl.asio.AsioConnection;
import hu.net.impl.asio.AsioRunner;
import hu.net.impl.asio.AsioType;
import hu.net.NetType;
import hu.Util;

import "hu/net/NetMacro.hpp";


namespace hu {

// Asio를 사용해서 구현한 클라이언트 클래스
export class AsioClient final : public ClientBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AsioClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit AsioClient(
        const ClientConfigInfo& config
    ) :
        ClientBase { config },
        runner_    { std::make_shared<AsioRunner>( config.run_thread ) }
    {
        connection_ = std::make_shared<AsioConnection>( config.connection, runner_ );
    }

    ~AsioClient() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AsioClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Start() override
    {
        if ( Connect() == false )
        {
            HU_NET_ERROR( _T( "연결 시도 실패 (ClientId = {})" ),
                config_.id );
            return false;
        }

        if ( runner_->Run() == false )
        {
            HU_NET_ERROR( _T( "실행기 실행 실패 (ClientId = {})" ),
                config_.id );
            return false;
        }

        return true;
    }

    virtual void Stop() override
    {
        runner_->Stop();
    }

    virtual bool Connect() override
    {
        if ( connection_->IsOpen() )
        {
            HU_NET_ERROR( _T( "이미 연결됨 (ClientId = {})" ),
                config_.id );
            return false;
        }

        AsioResolver resolver { runner_->GetContext() };

        AsioErrorCode ec;
        auto end_points = resolver.resolve( util::wstr_to_astr( config_.address ), std::to_string( config_.port ), ec );
        if ( ec )
        {
            HU_NET_ERROR( _T( "Resolve 실패 (ClientId = {}, {})" ),
                config_.id, util::asio_error_str( ec ) );
            return false;
        }

        const auto asio_conn { static_pointer_cast<AsioConnection>( connection_ ) };
        asio::async_connect( asio_conn->GetSocket(), end_points, [ this ]( const AsioErrorCode ec, const AsioEndPoint )
        {
            if ( ec )
            {
                HU_NET_ERROR( _T( "Connect 실패 (ClientId = {}, {})" ),
                    config_.id, util::asio_error_str( ec ) );

                if ( config_.connection.close_handler )
                    config_.connection.close_handler( config_.id, ConnectionCloseType::kFailToConnect );
                return;
            }

            if ( connection_->Open() == false )
            {
                HU_NET_ERROR( _T( "연결 Open 실패 (ClientId = {})" ),
                    config_.id );

                connection_->Close( ConnectionCloseType::kFailToOpen );
                return;
            }
        } );

        HU_NET_INFO( _T( "클라이언트 접속 준비 완료 (ClientId = {}, Address = {}, Port = {}, RunThread = {}, Header = {}, MaxMsgSize = {}, Crypt = {}, ThreadId = {})" ),
            config_.id,
            config_.address,
            config_.port,
            config_.run_thread,
            HeaderTypeInfo::ToStr( config_.connection.header ),
            config_.connection.max_msg_size,
            CryptImplTypeInfo::ToStr( config_.connection.crypt.crypt_impl ),
            util::get_thread_id() );
        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (AsioClient)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    AsioRunnerPtr runner_;
};

} // hu
