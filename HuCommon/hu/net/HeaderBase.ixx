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

// ��� ������ ���� ��� Ŭ����
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
    // �޽��� ����� �ܺ� ���ۿ� ����.
    virtual bool WriteTo(
        const Message& msg,
        Buffer&        buffer
    ) const = 0;

    // ��� ������ ���� ���۸� �غ��Ѵ�.
    virtual Buffer& ReadyBuffer() = 0;

    // ���� �޽����� �غ��Ѵ�.
    virtual bool ReadyMessage(
        Message& msg
    ) = 0;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (HeaderBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    // �޽��� ũ�⸦ �˻��Ѵ�.
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

// ��� ����ü Ŭ����
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

// �޽��� ũ�� 2����Ʈ ���
export using Header_Size2 = HeaderImpl<HeaderInfo_Size2>;

// �޽��� ũ�� 4����Ʈ ���
export using Header_Size4 = HeaderImpl<HeaderInfo_Size4>;

// �޽��� ũ�� 2����Ʈ, ���̵� 2����Ʈ ���
export using Header_Size2Id2 = HeaderImpl<HeaderInfo_Size2Id2>;

// �޽��� ũ�� 4����Ʈ, ���̵� 2����Ʈ ���
export using Header_Size4Id2 = HeaderImpl<HeaderInfo_Size4Id2>;

// �޽��� ũ�� 2����Ʈ, ���̵� 4����Ʈ ���
export using Header_Size2Id4 = HeaderImpl<HeaderInfo_Size2Id4>;

// �޽��� ũ�� 4����Ʈ, ���̵� 4����Ʈ ���
export using Header_Size4Id4 = HeaderImpl<HeaderInfo_Size4Id4>;

} // hu
