export module hu.net.Client;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Client)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;

import hu.log.Log;
import hu.net.ClientBase;
import hu.net.ClientConfig;
import hu.net.impl.asio.AsioClient;
import hu.net.Message;
import hu.net.NetType;
import hu.pattern.INoCopy;
import hu.serial.Serializer;

import "hu/net/NetMacro.hpp";


namespace hu {

// 클라이언트를 구현한 클래스
export class Client final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Client)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    Client() = default;
    ~Client() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Client)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 클라이언트를 시작한다.
    bool Start(
        const ClientConfigInfo& config
    )
    {
        if ( config.IsValid() == false )
        {
            HU_NET_ERROR( _T( "설정이 유효하지 않음 (ClientId = {})" ),
                config.id );
            return false;
        }

        config_ = config;

        impl_ = create_impl( config_ );
        if ( impl_ == nullptr )
            return false;

        return impl_->Start();
    }

    // 클라이언트를 중지한다.
    void Stop() const
    {
        impl_->Stop();
    }

    // 서버로 연결한다.
    bool Connect() const
    {
        return impl_->Connect();
    }

    // 연결을 닫는다.
    void Close() const
    {
        impl_->Close();
    }

    // 연결이 열려 있는지 검사한다.
    bool IsOpen() const
    {
        return impl_->IsOpen();
    }

    // 연결 아이디를 설정한다.
    void SetConnectId(
        const ConnectId conn_id
    ) const
    {
        impl_->SetConnectId( conn_id );
    }

    // 연결 아이디를 얻는다.
    ConnectId GetConnectId() const
    {
        return impl_->GetConnectId();
    }

    // 메시지를 보낸다.
    bool Send(
        Message& msg
    ) const
    {
        return impl_->Send( msg );
    }

    // 객체를 보낸다.
    template <typename T = Serializer>
    bool SendObj(
        const auto& obj
    ) const
    {
        Message msg;
        if ( msg.WriteObj<T>( obj ) == false )
            return false;

        return Send( msg );
    }


public:
    // 암호화 공개키를 얻는다.
    const Buffer& GetCryptPublicKey() const
    {
        return impl_->GetCryptPublicKey();
    }

    // 암호화 정보를 설정한다.
    void SetCrypt(
        const Buffer& other_public_key,
        const UInt32  seed
    ) const
    {
        impl_->SetCrypt( other_public_key, seed );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Client)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Impl = std::unique_ptr<ClientBase>;

private:
    static Impl create_impl(
        const ClientConfigInfo& config
    )
    {
        switch ( config.net_impl )
        {
        case NetImplType::kAsio:
            return std::make_unique<AsioClient>( config );
        }

        HU_NET_ERROR( _T( "생성을 지원하지 않음 (ClientId = {}, NetImpl = {})" ),
            config.id, NetImplTypeInfo::ToStr( config.net_impl ) );
        return nullptr;
    }

private:
    ClientConfigInfo config_;
    Impl             impl_;
};

} // hu
