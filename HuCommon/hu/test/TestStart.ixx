export module hu.test.TestStart;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import
////////////////////////////////////////////////////////////////////////////////////////////////////

import <functional>;

import hu.io.Console;
import hu.log.Log;
import hu.test.impl.TestAutoQueue;
import hu.test.impl.TestConsole;
import hu.test.impl.TestCrypt;
import hu.test.impl.TestExpression;
import hu.test.impl.TestLocalDB;
import hu.test.impl.TestLog;
import hu.test.impl.TestRandom;
import hu.test.impl.TestSerial;
import hu.test.impl.TestSingleton;
import hu.test.impl.TestTemp;
import hu.test.impl.TestUtil;
import hu.time.Timer;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestStart)
////////////////////////////////////////////////////////////////////////////////////////////////////

using TestFunc = std::function<bool()>;

void test(
    const Char* const test_name,
    const TestFunc&   test_func,
    const bool        is_skip = false
)
{
    if ( is_skip )
        return;

    HU_LOG_INFO( test_name, _T( "<< START >>" ) );

    if ( Timer timer; test_func() )
        HU_LOG_INFO( test_name, _T( "<< SUCCESS (Time = {} ms) >>" ), timer.GetMilliSeconds() );
    else
        HU_LOG_INFO( test_name, _T( "<< FAIL (Time = {} ms) >>" ), timer.GetMilliSeconds() );
}

#define HU_TEST( name ) test( _T( "test_"#name ), test_##name )

// 테스트를 시작한다.
export void start_test()
{
    // 로그 항목 별로 설정한다.
    LogConfigInfo log_conf;
    {
        // 비동기 구현 방식을 설정한다.
        //log_conf.async_impl = LogAsyncImplType::kMpsc;
        //log_conf.async_impl = LogAsyncImplType::kLockFree;

        // 콘솔 로그를 설정한다.
        {
            // 사용 여부를 설정한다.
            log_conf.console.use = true;
        }

        // 파일 로그를 설정한다.
        {
            // 사용 여부를 설정한다.
            log_conf.file.use = true;

            // 로그 파일 이름을 설정한다.
            log_conf.file.name = _T( "TestLog" );

            // 줄개수가 설정값을 넘으면 새 파일에 이어서 쓴다.
            log_conf.file.max_line = 1'000;
        }
    }

    // 로그 실행기를 사용해서 로그 인스턴스를 초기화 하고 자동으로 정리한다.
    // 로그 사용전에 로그를 사용할 범위에서 한번만 선언하면 된다.
    const LogRunner log_runner { log_conf };

    //HU_TEST( temp );
    //HU_TEST( expression );
    //HU_TEST( console );
    //HU_TEST( singleton );
    //HU_TEST( util );
    //HU_TEST( auto_queue );
    //HU_TEST( log );
    //HU_TEST( serial );
    //HU_TEST( random );
    //HU_TEST( crypt );
    HU_TEST( local_db );

    HU_LOG_INFO( _T( "Input" ), _T( "아무키나 누르면 종료 합니다." ) );
    Console::Inst().Wait();
}
