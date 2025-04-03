export module hu.net.HeaderBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (HeaderBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.ConnectionConfig;
import hu.net.HeaderInfo;
import hu.net.Message;
import hu.net.NetType;
import hu.pattern.INoCopy;
import hu.Type;
import hu.Util;


namespace hu {

// 헤더 구현을 위한 기반 클래스
export class HeaderBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (HeaderBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit HeaderBase(
        const ConnectionConfigInfo& config
    ) :
        config_ { config }
    {
    }

    virtual ~HeaderBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (HeaderBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 메시지 헤더를 외부 버퍼에 쓴다.
    virtual bool WriteTo(
        const Message& msg,
        Buffer&        buffer
    ) const = 0;

    // 헤더 정보를 읽을 버퍼를 준비한다.
    virtual Buffer& ReadyBuffer() = 0;

    // 받을 메시지를 준비한다.
    virtual bool ReadyMessage(
        Message& msg
    ) = 0;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (HeaderBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    // 메시지 크기를 검사한다.
    bool check_message_size(
        const Size msg_size
    ) const
    {
        return ( msg_size > 0 ) && ( msg_size <= config_.max_msg_size );
    }

protected:
    const ConnectionConfigInfo& config_;
    Buffer                      buffer_;
};

// 헤더 구현체 클래스
export template <typename T>
class HeaderImpl final : public HeaderBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (HeaderImpl)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit HeaderImpl(
        const ConnectionConfigInfo& config
    ) :
        HeaderBase { config }
    {
    }

    ~HeaderImpl() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (HeaderImpl)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool WriteTo( const Message& msg, Buffer& buffer ) const override
    {
        if ( check_message_size( msg.GetSize() ) == false )
            return false;

        const T info { msg };
        util::copy_to_buffer( info, buffer );
        return true;
    }

    virtual Buffer& ReadyBuffer() override
    {
        buffer_.clear();
        buffer_.resize( sizeof( T ) );
        return buffer_;
    }

    virtual bool ReadyMessage( Message& msg ) override
    {
        if ( util::copy_to_info( buffer_, info_ ) == false )
            return false;

        const auto size = info_.GetMessageSize();
        if ( check_message_size( size ) == false )
            return false;

        msg.Init( info_.GetMessageId(), size );
        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (HeaderImpl)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    T info_;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (HeaderBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 메시지 크기 2바이트 헤더
export using Header_Size2 = HeaderImpl<HeaderInfo_Size2>;

// 메시지 크기 4바이트 헤더
export using Header_Size4 = HeaderImpl<HeaderInfo_Size4>;

// 메시지 크기 2바이트, 아이디 2바이트 헤더
export using Header_Size2Id2 = HeaderImpl<HeaderInfo_Size2Id2>;

// 메시지 크기 4바이트, 아이디 2바이트 헤더
export using Header_Size4Id2 = HeaderImpl<HeaderInfo_Size4Id2>;

// 메시지 크기 2바이트, 아이디 4바이트 헤더
export using Header_Size2Id4 = HeaderImpl<HeaderInfo_Size2Id4>;

// 메시지 크기 4바이트, 아이디 4바이트 헤더
export using Header_Size4Id4 = HeaderImpl<HeaderInfo_Size4Id4>;

} // hu
