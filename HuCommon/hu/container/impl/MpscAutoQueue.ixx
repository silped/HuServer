export module hu.container.impl.MpscAutoQueue;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (MpscAutoQueue)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <condition_variable>;
import <mutex>;
import <queue>;
import <thread>;

import hu.container.AutoQueueBase;


namespace hu {

// 멀티 스레드에서 자유롭게 큐에 넣으면, 단일 자체 스레드에서 꺼내서 처리하는 멀티 생산자 싱글 소비자 모델 큐
export template <typename T>
class MpscAutoQueue final : public AutoQueueBase<T>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MpscAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    MpscAutoQueue() = default;
    ~MpscAutoQueue() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MpscAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Push(
        const T& elem
    ) override
    {
        {
            std::lock_guard<Mutex> lock { mutex_ };
            impl_.push( elem );
        }
        cond_.notify_one();
        return true;
    }

    virtual bool Push(
        T&& elem
    ) override
    {
        {
            std::lock_guard<Mutex> lock { mutex_ };
            impl_.push( std::move( elem ) );
        }
        cond_.notify_one();
        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (MpscAutoQueue)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Super = AutoQueueBase<T>;
    using Impl  = std::queue<T>;
    using Mutex = std::mutex;
    using Lock  = std::unique_lock<Mutex>;

private:
    virtual void start_impl() override
    {
        Super::thread_ = std::jthread { [ this ]
        {
            T elem;
            while ( pop_impl( elem ) )
                Super::handler_( elem );
        } };
    }

    virtual void stop_impl() override
    {
        cond_.notify_all();
    }

private:
    bool pop_impl( T& elem )
    {
        Lock lock { mutex_ };

        // 비어 있는 동안 대기 한다.
        if ( impl_.empty() )
        {
            cond_.wait( lock );

            // 깨어 났는데도 비어 있으면 종료한다.
            if ( impl_.empty() )
                return false;
        }

        elem = impl_.front();
        impl_.pop();

        return true;
    }

private:
    Impl                    impl_;
    Mutex                   mutex_;
    std::condition_variable cond_;
};

} // hu
