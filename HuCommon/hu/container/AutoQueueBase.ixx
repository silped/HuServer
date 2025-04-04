export module hu.container.AutoQueueBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AutoQueueBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <functional>;
import <thread>;
import <utility>;

import hu.pattern.INoCopy;


namespace hu {

// 멀티 스레드에서 자유롭게 큐에 넣으면, 단일 자체 스레드에서 꺼내서 처리하는 멀티 생산자 싱글 소비자 모델 큐
export template <typename T>
class AutoQueueBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AutoQueueBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    AutoQueueBase()
    {
    }

    virtual ~AutoQueueBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AutoQueueBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 처리기
    using Handler = std::function<void( const T& )>;

public:
    // 큐에 넣는다.
    virtual bool Push(
        const T& elem
    ) = 0;

    // 큐에 넣는다.
    virtual bool Push(
        T&& elem
    ) = 0;

public:
    // 처리를 시작한다.
    void Start(
        const Handler& handler
    )
    {
        handler_ = handler;

        start_impl();
    }

    // 처리를 시작한다.
    void Start(
        Handler&& handler
    )
    {
        handler_ = std::move( handler );

        start_impl();
    }

    // 대기를 중지한다.
    void Stop()
    {
        stop_impl();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (AutoQueueBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    using Thread = std::jthread;

protected:
    // 구현체 start를 구현한다.
    virtual void start_impl() = 0;

    // 구현체 stop을 구현한다.
    virtual void stop_impl() = 0;

protected:
    Handler handler_;
    Thread  thread_;
};

} // hu
