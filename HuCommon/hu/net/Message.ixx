export module hu.net.Message;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Message)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.net.NetType;
import hu.serial.Serializer;
import hu.Type;


namespace hu {

// 전송할 메시지를 구현한 클래스
export class Message final
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Message)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    Message() = default;
    ~Message() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Message)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 초기화 한다.
    void Init(
        const MessageId msg_id,
        const Size      size
    )
    {
        id_ = msg_id;

        buffer_.clear();
        buffer_.resize( size );
    }

    // 초기화 한다.
    void Init(
        const MessageId msg_id,
        const Buffer&   buffer
    )
    {
        id_     = msg_id;
        buffer_ = buffer;
    }

    // 아이디를 얻는다.
    MessageId GetId() const
    {
        return id_;
    }

    // 크기를 얻는다.
    Size GetSize() const
    {
        return buffer_.size();
    }

    // 버퍼를 얻는다.
    Buffer& GetBuffer()
    {
        return buffer_;
    }

    // 대상 버퍼에 쓴다.
    bool WriteTo(
        Buffer& buffer
    ) const
    {
        if ( buffer_.empty() )
            return false;

        buffer.insert( buffer.end(), buffer_.cbegin(), buffer_.cend() );
        return true;
    }

    // 객체를 쓴다.
    template <typename T = Serializer>
    bool WriteObj(
        const auto& obj
    )
    {
        id_ = obj.kType;

        buffer_.reserve( T::GetWriteSize( obj ) );
        return T::Write( obj, buffer_ );
    }

    // 객체를 읽는다.
    template <typename T = Serializer>
    bool ReadObj(
        auto& obj
    ) const
    {
        if ( id_ )
        {
            if ( id_ != obj.kType )
                return false;
        }

        return T::Read( buffer_, obj );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Message)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    MessageId id_ { 0 };
    Buffer    buffer_;
};

} // hu
