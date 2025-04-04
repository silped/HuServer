export module hu.net.Header;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Header)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;

import hu.net.ConnectionConfig;
import hu.net.HeaderBase;
import hu.net.Message;
import hu.net.NetType;
import hu.pattern.INoCopy;
import hu.Type;


namespace hu {

// 헤더를 구현한 클래스
export class Header final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Header)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit Header(
        const ConnectionConfigInfo& config
    ) :
        config_ { config },
        impl_   { create_impl( config ) }
    {
    }

    ~Header()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Header)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 메시지 헤더를 외부 버퍼에 쓴다.
    bool WriteTo(
        const Message& msg,
        Buffer&        buffer
    ) const
    {
        return impl_->WriteTo( msg, buffer );
    }

    // 헤더 정보를 읽을 버퍼를 준비한다.
    Buffer& ReadyBuffer()
    {
        return impl_->ReadyBuffer();
    }

    // 메시지를 받을 준비한다.
    bool ReadyMessage(
        Message& msg
    )
    {
        return impl_->ReadyMessage( msg );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Header)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Impl = std::unique_ptr<HeaderBase>;

private:
    static Impl create_impl(
        const ConnectionConfigInfo& config
    )
    {
        switch ( config.header )
        {
        case HeaderType::kSize2:
            return std::make_unique<Header_Size2>( config );
        case HeaderType::kSize4:
            return std::make_unique<Header_Size4>( config );
        case HeaderType::kSize2Id2:
            return std::make_unique<Header_Size2Id2>( config );
        case HeaderType::kSize4Id2:
            return std::make_unique<Header_Size4Id2>( config );
        case HeaderType::kSize2Id4:
            return std::make_unique<Header_Size2Id4>( config );
        case HeaderType::kSize4Id4:
            return std::make_unique<Header_Size4Id4>( config );
        }

        return std::make_unique<Header_Size2>( config );
    }

private:
    const ConnectionConfigInfo& config_;
    Impl                        impl_;
};

} // hu
