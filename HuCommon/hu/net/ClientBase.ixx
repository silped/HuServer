export module hu.net.ClientBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ClientBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.ClientConfig;
import hu.net.ConnectionBase;
import hu.net.Message;
import hu.net.NetType;
import hu.pattern.INoCopy;
import hu.Type;


namespace hu {

// 클라이언트 구현체를 위한 기반 클래스
export class ClientBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ClientBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit ClientBase(
        const ClientConfigInfo& config
    ) :
        config_ { config }
    {
    }

    virtual ~ClientBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ClientBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 클라이언트를 시작한다.
    virtual bool Start() = 0;

    // 클라이언트를 중지한다.
    virtual void Stop() = 0;

    // 서버로 연결한다.
    virtual bool Connect() = 0;

public:
    // 연결을 닫는다.
    void Close() const
    {
        connection_->Close();
    }

    // 연결이 열려 있는지 검사한다.
    bool IsOpen() const
    {
        return connection_->IsOpen();
    }

    // 연결 아이디를 설정한다.
    void SetConnectId(
        const ConnectId conn_id
    ) const
    {
        connection_->SetId( conn_id );
    }

    // 연결 아이디를 얻는다.
    ConnectId GetConnectId() const
    {
        return connection_->GetId();
    }

    // 메시지를 보낸다.
    bool Send(
        Message& msg
    ) const
    {
        return connection_->Send( msg );
    }

public:
    // 암호화 공개키를 얻는다.
    const Buffer& GetCryptPublicKey() const
    {
        return connection_->GetCryptPublicKey();
    }

    // 암호화 정보를 설정한다.
    void SetCrypt(
        const Buffer& other_public_key,
        const UInt32  seed
    ) const
    {
        connection_->CreateCryptSharedKey( other_public_key );
        connection_->CreateCryptNonce( seed );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (ClientBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    const ClientConfigInfo& config_;
    ConnectionPtr           connection_;
};

} // hu
