export module hu.test.impl.TestUtil;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestUtil)
////////////////////////////////////////////////////////////////////////////////////////////////////

import "hu/Core.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestUtil)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_util()
{
    // 문자열을 배열에 복사한다.
    Char chars[ 256 ] = { 0 };
    util::str_to_chars( _T( "가나다라" ), chars );
    HU_LOG_NDEBUG( _T( "문자열 배열 복사 = {}" ), chars );

    // 현재 시스템 시간 문자열을 얻는다.
    HU_LOG_NDEBUG( _T( "Local Time = {}" ), util::get_local_time_str() );

    // 현재 시스템 사용자 지정 시간 문자열을 얻는다.
    HU_LOG_NDEBUG( _T( "Local Time Format = {}" ), util::get_local_time_str( _T( "{:%Y-%m-%d_%H-%M-%OS}" ) ) );

    // 현재 UTC 시간 문자열을 얻는다.
    HU_LOG_NDEBUG( _T( "UTC Time = {}" ), util::get_utc_time_str() );

    // 현재 UTC 사용자 지정 시간 문자열을 얻는다.
    HU_LOG_NDEBUG( _T( "UTC Time Format = {}" ), util::get_utc_time_str( _T( "{:%Y-%m-%d_%H-%M-%OS}" ) ) );

    // 현재 프로세스 실행 경로를 얻는다.
    HU_LOG_NDEBUG( util::get_cur_path_str() );

    // 현재 스레드 아이디를 얻는다.
    HU_LOG_NDEBUG( _T( "Thread Id = {}" ), util::get_thread_id() );

    // WIDE 문자열로 변환한다.
    HU_ASSERT_R( util::astr_to_wstr( "Test" ) == L"Test" );
    HU_ASSERT_R( util::astr_to_wstr( "가나다라" ) == L"가나다라" );

    // ANSI 문자열로 변환한다.
    HU_ASSERT_R( util::wstr_to_astr( L"Test" ) == "Test" );
    HU_ASSERT_R( util::wstr_to_astr( L"가나다라" ) == "가나다라" );

    return true;
}
