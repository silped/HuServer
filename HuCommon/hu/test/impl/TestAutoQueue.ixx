export module hu.test.impl.TestAutoQueue;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestAutoQueue)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.container.impl.LockFreeAutoQueue;
import hu.container.impl.MpscAutoQueue;
import hu.io.Console;
import hu.log.Log;
import hu.Type;
import hu.Util;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Struct (TestAutoQueue)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 큐에 넣을 객체
struct Info
{
    void Test() const
    {
        HU_LOG_NDEBUG( _T( "Test (ThreadId = {}, Id = {})" ), util::get_thread_id(), id );
    }

    Int32 id { 0 };
};

HU_STATIC_ASSERT( PodType<Info> );


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestAutoQueue)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_auto_queue()
{
    using AutoQueue = MpscAutoQueue<Info>;
    //using AutoQueue = LockFreeAutoQueue<Info>;

    // 큐를 생성하고 비동기로 실행한다.
    AutoQueue queue;
    queue.Start( []( const Info& obj )
    {
        obj.Test();
    } );

    // 큐에 객체를 넣는다.
    auto push_func = [ &queue ]( const Int32 id, const Int32 count = 10 )
    {
        for ( auto i = id; i < ( id + count ); ++i )
        {
            HU_LOG_NDEBUG( _T( "Push (ThreadId = {}, Id = {})" ), util::get_thread_id(), i );
            queue.Push( Info { .id = i } );
        }
    };

    // 서로 다른 스레드에서 객체를 생성해서 큐에 넣고, 비동기로 처리 됨을 확인한다.
    String input;
    while ( input != _T( "q" ) )
    {
        HU_LOG_NINFO( _T( "Command (1 = Other thread push id 100, 2 = Other thread push id 200, 3 = Main thread push id 300, q = Quit)" ) );
        input = Console::Inst().ReadLn();

        if ( input == _T( "1" ) )
        {
            auto push_thread = std::jthread { push_func, 100 };
        }
        else if ( input == _T( "2" ) )
        {
            auto push_thread = std::jthread { push_func, 200 };
        }
        else if ( input == _T( "3" ) )
        {
            push_func( 300 );
        }
    }

    // 큐를 중지한다.
    queue.Stop();

    return true;
}
