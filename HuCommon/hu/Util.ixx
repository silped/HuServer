export module hu.Util;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Util)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <ctime>;

import <array>;
import <chrono>;
import <filesystem>;
import <format>;
import <locale>;
import <string>;
import <thread>;
import <vector>;

import hu.Type;

import "hu/Macro.hpp";


namespace hu::util {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (Util)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 가변 인자 형식 문자열을 얻는다.
export String get_format_str(
    const StringView str,
    const auto& ...  args
)
{
    return std::vformat( str, std::make_wformat_args( args ... ) );
}

// 시간 문자열을 얻는다.
template <auto kBufferSize = 32>
String get_time_str(
    const auto&      func,
    const StringView fmt
)
{
    const auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

    tm ts;
    if ( func( &ts, &now ) )
        return String();

    Char buf[ kBufferSize ] { 0 };
    std::wcsftime( buf, std::size( buf ), fmt.data(), &ts );

    return String( buf );
}

// 시간 포맷 문자열을 얻는다.
const String& get_time_fmt_str()
{
    static const String kStr { _T( "%F %T" ) };
    return kStr;
}

// 현재 로컬 시간 문자열을 얻는다.
export String get_local_time_str(
    const StringView fmt = get_time_fmt_str()
)
{
    return get_time_str( _localtime64_s, fmt );
}

// 현재 UTC 시간 문자열을 얻는다.
export String get_utc_time_str(
    const StringView fmt = get_time_fmt_str()
)
{
    return get_time_str( _gmtime64_s, fmt );
}

// 현재 실행 경로 문자열을 얻는다.
export String get_cur_path_str()
{
    return std::filesystem::current_path().wstring();
}

// 현재 스레드 아이디를 얻는다.
export UInt32 get_thread_id()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    UInt32 id { 0 };
    ss >> id;
    return id;
}

// 현재 스레드를 대기 시킨다.
export void sleep_thread(
    const Size msec
)
{
    std::this_thread::sleep_for( std::chrono::milliseconds( msec ) );
}

// 전역 로케일을 설정한다.
export void set_locale(
    const std::string_view loc_name = ""
)
{
    setlocale( LC_ALL, loc_name.data() );
}

// ANSI 문자열을 WIDE 문자열로 변환한다.
export std::wstring astr_to_wstr(
    const std::string_view astr
)
{
    std::vector<wchar_t> buf( astr.size() + 1 );
    mbstowcs( buf.data(), astr.data(), buf.size() );
    return std::wstring( buf.data() );
}

// WIDE 문자열을 ANSI 문자열로 변환한다.
export std::string wstr_to_astr(
    const std::wstring_view wstr
)
{
    std::vector<char> buf( wstr.size() * 2 + 1 );
    wcstombs( buf.data(), wstr.data(), buf.size() );
    return std::string( buf.data() );
}

// 구조체를 버퍼로 복사한다.
export template <CopyableType T>
void copy_to_buffer(
    const T& info,
    Buffer&  buffer
)
{
    buffer.clear();
    buffer.resize( sizeof( T ) );
    std::memcpy( buffer.data(), &info, sizeof( T ) );
}

// 버퍼를 구조체로 복사한다.
export template <CopyableType T>
bool copy_to_info(
    const Buffer& buffer,
    T&            info )
{
    if ( buffer.size() < sizeof( T ) )
        return false;

    std::memcpy( &info, buffer.data(), sizeof( T ) );
    return true;
}

} // hu::util
