export module hu.test.impl.TestAMQP;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestAMQP)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.amqp.AMQP;

import "hu/Core.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestAMQP)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_amqp()
{
    AMQP amqp;

    // 메시지 브로커 접속 정보를 설정한다.
    auto& config = amqp.GetConfig();
    {
        config.host     = _T( "dingo-01.rmq.cloudamqp.com" );
        config.user     = _T( "zwienumj" );
        config.password = _T( "YwRAtRDjb4Uu-nFJdKb6monKSoeV28XO" );
        config.vhost    = _T( "zwienumj" );

        // 소비 처리기를 설정한다.
        config.AddConsumeHandler( _T( "test_queue" ), []( const AMQPConfigInfo::ConsumeResultInfo& result )
        {
            HU_LOG_NDEBUG( _T( "AMQP 메시지 소비 ({})" ), result.ToStr() );
        } );
    }

    // AMQP 처리기를 시작한다.
    amqp.Start();

    // 메시지 브로커에 발행을 요청한다.
    amqp.Publish( _T( "test_exchange" ), _T( "test_key" ), _T( "Test1" ) );
    amqp.Publish( _T( "test_exchange" ), _T( "test_key" ), _T( "Test2" ) );
    amqp.Publish( _T( "test_exchange" ), _T( "test_key" ), _T( "Test3" ) );
    amqp.Publish( _T( "test_exchange" ), _T( "test_key" ), _T( "Test4" ) );

    // 소비를 기다리고 종료한다.
    Console::Inst().Wait();
    amqp.Stop();

    return true;
}
