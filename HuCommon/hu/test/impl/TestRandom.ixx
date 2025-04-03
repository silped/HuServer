export module hu.test.impl.TestRandom;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestRandom)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.rand.Random;
import hu.Type;
import hu.Util;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestRandom)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_random()
{
    RandomInt rand { 1, 10000 };

    for ( auto i = 1; i <= 10; ++i )
    {
        HU_LOG_NDEBUG( _T( "{} 회차 값 = {}" ), i, rand.Generate() );
        HU_LOG_NDEBUG( _T( "{} 회차 함수값 = {}" ), i, util::get_random_int( 1, 10000 ) );
    }

    FixRandomInt fix_rand1 { 100001, 1, 10000 };
    FixRandomInt fix_rand2 { 100001, 1, 10000 };

    for ( auto i = 1; i <= 10; ++i )
    {
        HU_LOG_NDEBUG( _T( "{} 회차 고정1 난수값 = {}" ), i, fix_rand1.Generate() );
        HU_LOG_NDEBUG( _T( "{} 회차 고정2 난수값 = {}" ), i, fix_rand2.Generate() );
    }

    return true;
}
