export module hu.net.impl.asio.AsioConnection;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AsioConnection)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.net.ConnectionBase;
import hu.net.impl.asio.AsioRunner;
import hu.net.impl.asio.AsioType;

import "hu/net/NetMacro.hpp";


namespace hu {

// Asio를 사용해서 구현한 연결 클래스
export class AsioConnection final : public ConnectionBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AsioConnection)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    AsioConnection(
        const ConnectionConfigInfo& config,
        const AsioRunnerPtr&        runner
    ) :
        ConnectionBase { config },
        runner_        { runner },
        socket_        { runner->GetContext() }
    {
    }

    AsioConnection(
        const ConnectionConfigInfo& config,
        const AsioRunnerPtr&        runner,
        AsioSocket&&                socket
    ) :
        ConnectionBase { config },
        runner_        { runner },
        socket_        { std::move( socket ) }
    {
    }

    virtual ~AsioConnection()
    {
        if ( socket_.is_open() )
            socket_.close();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AsioConnection)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    AsioSocket& GetSocket()
    {
        return socket_;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (AsioConnection)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    virtual bool open_impl() override
    {
        AsioErrorCode ec;

        if ( config_.send_buffer_size > 0 )
        {
            socket_.set_option( AsioSocketBase::send_buffer_size { config_.send_buffer_size }, ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "SendBufferSize 옵션 설정 실패 (ConnId = {}, {})" ),
                    GetId(), util::asio_error_str( ec ) );
                return false;
            }
        }

        if ( config_.recv_buffer_size > 0 )
        {
            socket_.set_option( AsioSocketBase::receive_buffer_size { config_.recv_buffer_size }, ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "ReceiveBufferSize 옵션 설정 실패 (ConnId = {}, {})" ),
                    GetId(), util::asio_error_str( ec ) );
                return false;
            }
        }

        if ( config_.linger_sec > 0 )
        {
            socket_.set_option( AsioSocketBase::linger { true, config_.linger_sec }, ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "Linger 옵션 설정 실패 (ConnId = {}, {})" ),
                    GetId(), util::asio_error_str( ec ) );
                return false;
            }
        }

        if ( config_.use_no_delay )
        {
            socket_.set_option( AsioTcp::no_delay { config_.use_no_delay }, ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "NoDelay 옵션 설정 실패 (ConnId = {}, {})" ),
                    GetId(), util::asio_error_str( ec ) );
                return false;
            }
        }

        if ( config_.use_keep_alive )
        {
            socket_.set_option( AsioSocketBase::keep_alive { config_.use_keep_alive }, ec );
            if ( ec )
            {
                HU_NET_ERROR( _T( "KeepAlive 옵션 설정 실패 (ConnId = {}, {})" ),
                    GetId(), util::asio_error_str( ec ) );
                return false;
            }
        }

        // 소켓 설정이 끝났으면 헤더 받기를 시작한다.
        start_receive_header();
        return true;
    }

    virtual void close_impl() override
    {
        AsioErrorCode ec;
        socket_.close( ec );
        if ( ec )
        {
            HU_NET_ERROR( _T( "소켓 닫기 실패 (ConnId = {}, {})" ),
                GetId(), util::asio_error_str( ec ) );
        }
    }

    virtual bool send_impl( const Buffer& buffer ) override
    {
        asio_impl::post( runner_->GetContext(), [ this, buffer ]
        {
            start_send( buffer );
        } );

        return true;
    }

private: 
    // 에러를 무시할지 검사한다.
    bool is_skip_error( const AsioErrorCode ec )
    {
        switch ( ec.value() )
        {
        case asio_error::operation_aborted:
            close_by_self( ConnectionCloseType::kCancel );
            return true;
        case asio_error::connection_reset:
            close_by_self( ConnectionCloseType::kCloseByOther );
            return true;
        }

        return false;
    }

    // 메시지 보내기를 시작한다.
    void start_send( const Buffer& buffer )
    {
        send_buffer_ = buffer;

        asio_impl::async_write( socket_, asio_impl::buffer( send_buffer_ ), [ this ]( const AsioErrorCode ec, const Size size )
        {
            if ( ec )
            {
                if ( is_skip_error( ec ) )
                    return;

                HU_NET_ERROR( _T( "메시지 보내기 실패 (ConnId = {}, WriteSize = {}, {})" ),
                    GetId(), size, util::asio_error_str( ec ) );

                close_by_self( ConnectionCloseType::kFailToSend );
            }
        } );
    }

    // 헤더 받기를 시작한다.
    void start_receive_header()
    {
        asio_impl::async_read( socket_, asio_impl::buffer( header_.ReadyBuffer() ), [ this ]( const AsioErrorCode ec, const Size size )
        {
            if ( ec )
            {
                if ( is_skip_error( ec ) )
                    return;

                HU_NET_ERROR( _T( "헤더 받기 실패 (ConnId = {}, ReadSize = {}, {})" ),
                    GetId(), size, util::asio_error_str( ec ) );

                close_by_self( ConnectionCloseType::kFailToRecvHeader );
                return;
            }

            // 받은 헤더로 메시지를 받을 준비한다.
            if ( header_.ReadyMessage( recv_msg_ ) == false )
            {
                HU_NET_ERROR( _T( "메시지 받을 준비 실패 (ConnId = {}, MsgId = {}, MsgSize = {})" ),
                    GetId(), recv_msg_.GetId(), recv_msg_.GetSize() );

                close_by_self( ConnectionCloseType::kFailToReadyMsg );
                return;
            }

            // 성공 했으면 메시지 받기를 시작한다.
            start_receive_message();
        } );
    }

    // 메시지 받기를 시작한다.
    void start_receive_message()
    {
        asio_impl::async_read( socket_, asio_impl::buffer( recv_msg_.GetBuffer() ), [ this ]( const AsioErrorCode ec, const Size size )
        {
            if ( ec )
            {
                if ( is_skip_error( ec ) )
                    return;

                HU_NET_ERROR( _T( "메시지 받기 실패 (ConnId = {}, ReadSize = {}, MsgId = {}, MsgSize = {}, {})" ),
                    GetId(), size, recv_msg_.GetId(), recv_msg_.GetSize(), util::asio_error_str( ec ) );

                close_by_self( ConnectionCloseType::kFailToRecvMsg );
                return;
            }

            // 메시지를 복호화 한다.
            crypt_.Decrypt( recv_msg_.GetBuffer() );

            // 메시지 처리기를 호출한다.
            if ( config_.HandleMessage( GetId(), recv_msg_ ) == false )
            {
                HU_NET_ERROR( _T( "메시지 처리 실패 (ConnId = {}, MsgId = {}, MsgSize = {})" ),
                    GetId(), recv_msg_.GetId(), recv_msg_.GetSize() );

                close_by_self( ConnectionCloseType::kFailToHandleMsg );
                return;
            }

            // 다음 메시지 헤더 읽기를 시작한다.
            start_receive_header();
        } );
    }

private:
    AsioRunnerPtr runner_;
    AsioSocket    socket_;
    Buffer        send_buffer_;
};

} // hu
