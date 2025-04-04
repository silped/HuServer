export module hu.net.Server;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Server)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;

import hu.log.Log;
import hu.net.impl.asio.AsioServer;
import hu.net.Message;
import hu.net.NetType;
import hu.net.ServerBase;
import hu.net.ServerConfig;
import hu.pattern.INoCopy;
import hu.serial.Serializer;
import hu.Type;

import "hu/net/NetMacro.hpp";


namespace hu {

// 서버를 구현한 클래스
export class Server final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Server)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    Server() = default;
    ~Server() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Server)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 서버를 시작한다.
    bool Start(
        const ServerConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_NET_ERROR( _T( "설정이 유효하지 않음 (ServerId = {})" ),
                config.id );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
            return false;

        return impl_->Start();
    }

    // 서버를 중지한다.
    void Stop() const
    {
        impl_->Stop();
    }

public:
    // 연결 개수를 얻는다.
    Size GetConnectionCount() const
    {
        return impl_->GetConnectionCount();
    }

    // 연결을 찾아서 닫는다.
    void Close(
        const ConnectId           conn_id,
        const ConnectionCloseType close_type
    ) const
    {
        impl_->Close( conn_id, close_type );
    }

    // 연결을 찾아서 메시지를 보낸다.
    bool Send(
        const ConnectId conn_id,
        Message&        msg
    ) const
    {
        return impl_->Send( conn_id, msg );
    }

    // 연결을 찾아서 객체를 보낸다.
    template <typename T = Serializer>
    bool SendObj(
        const ConnectId conn_id,
        const auto&     obj
    ) const
    {
        Message msg;
        if ( msg.WriteObj<T>( obj ) == false )
            return false;

        return impl_->Send( conn_id, msg );
    }

public:
    // 연결을 찾아서 암호화 공개키를 얻는다.
    const Buffer& GetCryptPublicKey(
        const ConnectId conn_id
    ) const
    {
        return impl_->GetCryptPublicKey( conn_id );
    }

    // 연결을 찾아서 암호화 정보를 생성한다.
    bool SetCrypt(
        const ConnectId conn_id,
        const Buffer&   other_public_key,
        const UInt32    seed
    ) const
    {
        return impl_->SetCrypt( conn_id, other_public_key, seed );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Server)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Impl = std::unique_ptr<ServerBase>;

private:
    static Impl create_impl(
        const ServerConfigInfo& config
    )
    {
        switch ( config.net_impl )
        {
        case NetImplType::kAsio:
            return std::make_unique<AsioServer>( config );
        }

        HU_NET_ERROR( _T( "생성을 지원하지 않음 (ServerId = {}, NetImpl = {})" ),
            config.id, NetImplTypeInfo::ToStr( config.net_impl ) );
        return nullptr;
    }

private:
    ServerConfigInfo config_;
    Impl             impl_;
};

} // hu
