export module hu.net.ServerBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ServerBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.ConnectionManager;
import hu.net.Message;
import hu.net.NetType;
import hu.net.ServerConfig;
import hu.pattern.INoCopy;
import hu.Type;


namespace hu {

// 서버 구현체를 위한 기반 클래스
export class ServerBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ServerBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit ServerBase(
        const ServerConfigInfo& config
    ) :
        config_       { config },
        conn_manager_ { config }
    {
    }

    virtual ~ServerBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ServerBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 서버를 시작한다.
    virtual bool Start() = 0;

    // 서버를 중지한다.
    virtual void Stop() = 0;

public:
    // 연결 개수를 얻는다.
    Size GetConnectionCount() const
    {
        return conn_manager_.GetCount();
    }

    // 연결을 찾아서 닫는다.
    void Close(
        const ConnectId           conn_id,
        const ConnectionCloseType close_type
    )
    {
        conn_manager_.Close( conn_id, close_type );
    }

    // 연결을 찾아서 메시지를 보낸다.
    bool Send(
        const ConnectId conn_id,
        Message&        msg
    ) const
    {
        const auto conn { conn_manager_.Find( conn_id ) };
        if ( conn == nullptr )
            return false;

        return conn->Send( msg );
    }

public:
    // 연결을 찾아서 암호화 공개키를 얻는다.
    const Buffer& GetCryptPublicKey(
        const ConnectId conn_id
    ) const
    {
        const auto conn { conn_manager_.Find( conn_id ) };
        if ( conn == nullptr )
        {
            static const Buffer empty;
            return empty;
        }

        return conn->GetCryptPublicKey();
    }

    // 연결을 찾아서 암호화 정보를 설정한다.
    bool SetCrypt(
        const ConnectId conn_id,
        const Buffer&   other_public_key,
        const UInt32    seed
    ) const
    {
        const auto conn { conn_manager_.Find( conn_id ) };
        if ( conn == nullptr )
            return false;

        conn->CreateCryptSharedKey( other_public_key );
        conn->CreateCryptNonce( seed );
        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (ServerBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    const ServerConfigInfo& config_;
    ConnectionManager       conn_manager_;
};

} // hu
