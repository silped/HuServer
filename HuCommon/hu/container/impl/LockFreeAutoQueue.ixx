export module hu.container.impl.LockFreeAutoQueue;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LockFreeAutoQueue)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <atomic>;
import <thread>;
import <utility>;

import hu.Util;
import hu.container.AutoQueueBase;
import hu.container.LockFreeQueue;


namespace hu {

// 멀티 스레드에서 자유롭게 큐에 넣으면, 단일 자체 스레드에서 비동기로 처리하는 락프리 큐
export template <typename ELEM_T, typename IMPL_T = LockFreeQueue<ELEM_T>>
class LockFreeAutoQueue final : public AutoQueueBase<ELEM_T>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LockFreeAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    LockFreeAutoQueue() = default;
    ~LockFreeAutoQueue() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (LockFreeAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Push(
        const ELEM_T& elem
    ) override
    {
        return impl_.push( elem );
    }

    virtual bool Push(
        ELEM_T&& elem
    ) override
    {
        return impl_.push( std::move( elem ) );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (LockFreeAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Super = AutoQueueBase<ELEM_T>;

private:
    virtual void start_impl() override
    {
        is_run_ = true;

        Super::thread_ = std::jthread { [ this ]
        {
            while ( is_run_ )
            {
                ELEM_T elem;
                while ( impl_.pop( elem ) )
                    Super::handler_( elem );

                // CPU 사용률을 줄인다.
                util::sleep_thread( 1 );
            }
        } };
    }

    virtual void stop_impl() override
    {
        is_run_ = false;
    }

private:
    IMPL_T            impl_;
    std::atomic<bool> is_run_ { false };
};

} // hu
