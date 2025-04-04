export module hu.net.impl.asio.AsioRunner;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AsioRunner)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;
import <thread>;

import hu.log.Log;
import hu.net.NetType;
import hu.net.impl.asio.AsioType;
import hu.pattern.INoCopy;

import "hu/net/NetMacro.hpp";


namespace hu {

// Asio 작업 실행기를 구현한 클래스
export class AsioRunner final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (AsioRunner)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit AsioRunner(
        const bool run_thread
    ) :
        run_thread_ { run_thread }
    {
    }

    ~AsioRunner() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (AsioRunner)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 컨텍스트를 얻는다.
    AsioContext& GetContext()
    {
        return context_;
    }

    // 작업을 실행하고 모든 작업이 완료될때까지 대기한다.
    bool Run()
    {
        if ( run_thread_ )
        {
            thread_ = Thread { [ this ]
            {
                run_impl();
            } };
        }
        else
        {
            return run_impl();
        }

        return true;
    }

    // 작업을 중지한다.
    void Stop()
    {
        context_.stop();

        if ( run_thread_ )
            thread_.join();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (AsioRunner)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Thread = std::jthread;

private:
    bool run_impl()
    {
        AsioErrorCode ec;
        context_.run( ec );

        if ( ec )
        {
            HU_NET_ERROR( _T( "실행기 실행 실패 ({})" ),
                util::asio_error_str( ec ) );
            return false;
        }

        return true;
    }

private:
    bool        run_thread_ { false };
    Thread      thread_;
    AsioContext context_;
};

export using AsioRunnerPtr = std::shared_ptr<AsioRunner>;

} // hu
