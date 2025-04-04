export module hu.net.ConnectionBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ConnectionBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <atomic>;

import hu.crypt.Crypt;
import hu.log.Log;
import hu.net.ConnectionConfig;
import hu.net.Header;
import hu.net.Message;
import hu.net.NetType;
import hu.pattern.INoCopy;
import hu.Type;

import "hu/net/NetMacro.hpp";


namespace hu {

// 연결 구현체를 위한 기반 클래스
export class ConnectionBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ConnectionBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    explicit ConnectionBase(
        const ConnectionConfigInfo& config
    ) :
        config_ { config },
        header_ { config },
        crypt_  { config_.crypt }
    {
    }

    virtual ~ConnectionBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ConnectionBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 연결 닫기 대리자
    using CloseDelegate = ConnectionConfigInfo::CloseHandler;

public:
    // 연결 아이디를 설정한다.
    void SetId(
        const ConnectId id
    )
    {
        id_ = id;
    }

    // 연결 아이디를 얻는다.
    ConnectId GetId() const
    {
        return id_;
    }

    // 연결 닫기 대리자를 설정한다.
    void SetCloseDelegate(
        const CloseDelegate& close_delegate
    )
    {
        close_delegate_ = close_delegate;
    }

    // 열려 있는지 검사한다.
    bool IsOpen() const
    {
        return open_;
    }

    // 연결을 연다.
    bool Open()
    {
        if ( open_ )
        {
            HU_NET_ERROR( _T( "연결이 이미 열림 (ConnId = {})" ),
                GetId() );
            return false;
        }

        if ( open_impl() == false )
        {
            HU_NET_ERROR( _T( "구현체 open 실패 (ConnId = {})" ),
                GetId() );
            return false;
        }

        open_ = true;

        if ( config_.open_handler )
            config_.open_handler( id_ );

        return true;
    }

    // 연결을 닫는다.
    void Close(
        const ConnectionCloseType close_type = ConnectionCloseType::kFunction
    )
    {
        if ( open_ == false )
        {
            HU_NET_ERROR( _T( "연결이 이미 닫힘 (ConnId = {}, CloseType = {})" ),
                GetId(), ConnectionCloseTypeInfo::ToStr( close_type ) );
            return;
        }

        close_impl();

        open_ = false;

        if ( config_.close_handler )
            config_.close_handler( id_, close_type );
    }

    // 메시지를 보낸다.
    bool Send(
        Message& msg
    )
    {
        if ( open_ == false )
        {
            HU_NET_ERROR( _T( "연결이 닫혀 있음 (ConnId = {}, MsgId = {}, MsgSize = {})" ),
                GetId(), msg.GetId(), msg.GetSize() );
            return false;
        }

        // 메시지를 암호화 한다.
        crypt_.Encrypt( msg.GetBuffer() );

        // 메시지를 버퍼에 쓴다.
        Buffer buffer;
        {
            if ( header_.WriteTo( msg, buffer ) == false )
            {
                HU_NET_ERROR( _T( "메시지 헤더 쓰기 실패 (ConnId = {}, MsgId = {}, MsgSize = {})" ),
                    GetId(), msg.GetId(), msg.GetSize() );
                return false;
            }

            if ( msg.WriteTo( buffer ) == false )
            {
                HU_NET_ERROR( _T( "메시지 쓰기 실패 (ConnId = {}, MsgId = {}, MsgSize = {})" ),
                    GetId(), msg.GetId(), msg.GetSize() );
                return false;
            }
        }
        return send_impl( buffer );
    }

public:
    // 암호화 공개키를 얻는다.
    const Buffer& GetCryptPublicKey() const
    {
        return crypt_.GetPublicKey();
    }

    // 암호화 공유키를 생성한다.
    void CreateCryptSharedKey(
        const Buffer& other_public_key
    )
    {
        crypt_.CreateSharedKey( other_public_key );
    }

    // 암호화 논스를 생성한다.
    void CreateCryptNonce(
        const UInt32 seed
    )
    {
        crypt_.CreateNonce( seed );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (ConnectionBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    // 구현체 open을 구현한다.
    virtual bool open_impl() = 0;

    // 구현체 close를 구현한다.
    virtual void close_impl() = 0;

    // 구현체 send를 구현한다.
    virtual bool send_impl(
        const Buffer& buffer
    ) = 0;

protected:
    // 연결 내부에서 연결을 닫는다.
    void close_by_self(
        const ConnectionCloseType close_type
    )
    {
        if ( close_delegate_ )
        {
            close_delegate_( id_, close_type );
            return;
        }

        Close( close_type );
    }

protected:
    const ConnectionConfigInfo& config_;
    std::atomic<ConnectId>      id_   { 0 };
    std::atomic<bool>           open_ { false };
    Header                      header_;
    Message                     recv_msg_;
    CloseDelegate               close_delegate_;
    Crypt                       crypt_;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (ConnectionBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

export using ConnectionPtr = std::shared_ptr<ConnectionBase>;

} // hu
