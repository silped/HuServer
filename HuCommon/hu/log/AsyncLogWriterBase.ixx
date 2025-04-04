export module hu.log.AsyncLogWriterBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import
////////////////////////////////////////////////////////////////////////////////////////////////////

import <functional>;
import <memory>;

import hu.container.AutoQueueBase;
import hu.container.impl.LockFreeAutoQueue;
import hu.container.impl.MpscAutoQueue;
import hu.log.LogWriterBase;


namespace hu {

// 비동기 로그 기록기 구현체를 위한 기반 클래스
export class AsyncLogWriterBase : public LogWriterBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AsyncLogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit AsyncLogWriterBase(
        const LogConfigInfo& config
    ) :
        LogWriterBase { config }
    {
    }

    virtual ~AsyncLogWriterBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AsyncLogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    void Start() override
    {
        impl_ = create_impl( config_ );
        impl_->Start( std::bind_front( &AsyncLogWriterBase::write_impl, this ) );
    }

    void Write(
        const LogMsgInfo& msg
    ) override
    {
        impl_->Push( msg );
    }

    void Stop() override
    {
        impl_->Stop();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (AsyncLogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Impl    = AutoQueueBase<LogMsgInfo>;
    using ImplPtr = std::unique_ptr<Impl>;

private:
    static ImplPtr create_impl(
        const LogConfigInfo& config
    )
    {
        using MpscImpl     = MpscAutoQueue<LogMsgInfo>;
        using LockFreeImpl = LockFreeAutoQueue<LogMsgInfo>;

        switch ( config.async_impl )
        {
        case LogAsyncImplType::kMpsc:
            return std::make_unique<MpscImpl>();
        case LogAsyncImplType::kLockFree:
            return std::make_unique<LockFreeImpl>();
        }

        return nullptr;
    }

private:
    ImplPtr impl_;
};

} // hu
