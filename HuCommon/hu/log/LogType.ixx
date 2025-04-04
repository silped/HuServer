export module hu.log.LogType;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LogType)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <stdlib.h>;

import hu.Type;

import "hu/Macro.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Enum (LogType)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 로그 종류
HU_CREATE_ENUM( LogType, Debug, Info, Warn, Error );

// 로그 비동기 구현 방식
HU_CREATE_ENUM( LogAsyncImplType, Mpsc, LockFree );


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Struct (LogType)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 로그 설정
export struct LogConfigInfo final
{
    // 콘솔 출력 설정
    struct ConsoleInfo final
    {
        bool use { true }; // 사용 여부
    };

    // 파일 기록 설정
    struct FileInfo final
    {
        bool   use      { true };          // 사용 여부
        String dir      { _T( "_log" ) };  // 파일 디렉터리
        String name     { _T( "HuLog" ) }; // 파일 이름
        Size   max_line { 10000 };         // 파일 당 최대 줄 수 넘어가면 새 파일 생성
    };

    ConsoleInfo      console;                                // 콘솔 출력 설정
    FileInfo         file;                                   // 파일 기록 설정
    LogAsyncImplType async_impl { LogAsyncImplType::kMpsc }; // 비동기 구현 방식
};

// 로그 메시지
export struct LogMsgInfo final
{
    static constexpr auto kMaxTime     = 32;
    static constexpr auto kMaxCategory = 16;
    static constexpr auto kMaxDesc     = 512;

    UInt64      id                       { 0 };              // 고유 식별자
    LogType     type                     { LogType::kInfo }; // 종류
    Char        time[ kMaxTime ]         { 0 };              // 생성 시간
    Char        category[ kMaxCategory ] { 0 };              // 분류
    Char        desc[ kMaxDesc ]         { 0 };              // 설명
    SrcLocation location;

    LogMsgInfo() = default;

    LogMsgInfo(
        const UInt64       _id,
        const LogType      _type,
        const StringView   _time,
        const StringView   _category,
        const StringView   _desc,
        const SrcLocation& _location
    ) :
        id       { _id },
        type     { _type },
        location { _location }
    {
        wcsncpy_s( time, std::size( time ), _time.data(), _TRUNCATE );
        wcsncpy_s( category, std::size( category ), _category.data(), _TRUNCATE );
        wcsncpy_s( desc, std::size( desc ), _desc.data(), _TRUNCATE );
    }
};

// POD 타입인지 검사한다.
HU_STATIC_ASSERT( PodType<LogMsgInfo> );

} // hu
