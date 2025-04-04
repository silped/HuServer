export module hu.io.Console;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Console)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <fcntl.h>;
import <io.h>;

import <array>;
import <iostream>;
import <syncstream>;

#ifdef _WIN32
import <windows.h>;
#endif

import hu.pattern.ISingleton;
import hu.Type;
import hu.Util;

import "hu/Macro.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Impl (Console)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 멀티 스레드에서 안전한 출력 스트림을 반환한다.
auto ostream()
{
    return std::wosyncstream( std::wcout );
}

// 표준 구현체
struct BaseImpl
{
    BaseImpl() = default;
    virtual ~BaseImpl() = default;

    virtual void SetTitle(
        const StringView str
    ) const
    {
        ostream() << _T( "\033]0;" ) << str << _T( "\007" );
    }
};

// Win 구현체
#ifdef _WIN32
struct WinImpl : public BaseImpl
{
    WinImpl()
    {
        HANDLE handle { GetStdHandle( STD_OUTPUT_HANDLE ) };
        DWORD  mode   { 0 };
        GetConsoleMode( handle, &mode );

        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode( handle, mode );
    }

    virtual void SetTitle(
        const StringView str
    ) const override
    {
        SetConsoleTitle( str.data() );
    }
};

using Impl = WinImpl;
#else
using Impl = BaseImpl;
#endif

// 콘솔창을 구현한 클래스
export class Console final : public ISingleton<Console>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Console)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    friend class ISingleton;

    Console()
    {
        _setmode( _fileno( stdout ), _O_U16TEXT );
        _setmode( _fileno( stderr ), _O_U16TEXT );
        _setmode( _fileno( stdin ), _O_U16TEXT );

        util::set_locale();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Console)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 창 제목을 설정한다.
    void SetTitle(
        const StringView str
    ) const
    {
        impl_.SetTitle( str );
    }

    // 한줄을 읽는다.
    String ReadLn() const
    {
        String str;
        std::getline( std::wcin, str );
        return str;
    }

    // 문자열을 출력한다.
    void Write(
        const StringView str
    ) const
    {
        ostream() << str;
    }

    // 가변 인자 형식 문자열을 출력한다.
    void Write(
        const StringView str,
        const auto& ...  args
    ) const
    {
        Write( util::get_format_str( str, args ... ) );
    }

    // 줄바꿈한다.
    void WriteLn() const
    {
        ostream() << std::endl;
    }

    // 문자열을 출력하고 줄바꿈한다.
    void WriteLn(
        const StringView str
    ) const
    {
        ostream() << str << std::endl;
    }

    // 가변 인자 형식 문자열을 출력하고 줄바꿈한다.
    void WriteLn(
        const StringView str,
        const auto& ...  args
    ) const
    {
        WriteLn( util::get_format_str( str, args ... ) );
    }

    // 문자열을 색상을 설정해서 출력하고 줄바꿈한다.
    void WriteLn(
        const ColorType  color,
        const StringView str
    ) const
    {
        static const Array<const String, ColorTypeInfo::kCount> strs
        {
            _T( "\033[30m" ),
            _T( "\033[31m" ),
            _T( "\033[32m" ),
            _T( "\033[33m" ),
            _T( "\033[34m" ),
            _T( "\033[35m" ),
            _T( "\033[36m" ),
            _T( "\033[37m" ),
        };
        static const String reset_str { _T( "\033[0m" ) };

        ostream() << strs[ ColorTypeInfo::ToNum( color ) ] << str << reset_str << std::endl;
    }

    // 가변 인자 형식 문자열을 색상을 설정해서 출력하고 줄바꿈한다.
    void WriteLn(
        const ColorType  color,
        const StringView str,
        const auto& ...  args
    ) const
    {
        WriteLn( color, util::get_format_str( str, args ... ) );
    }

    // 아무 입력이나 대기한다.
    void Wait() const
    {
        ReadLn();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Console)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    Impl impl_;
};

} // hu
