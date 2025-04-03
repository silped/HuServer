export module hu.test.impl.TestConsole;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestConsole)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.io.Console;
import hu.log.Log;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestConsole)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_console()
{
    Console& cs = Console::Inst();

    // 한줄에 모아서 출력한다.
    cs.Write( _T( "한줄, " ) );
    cs.Write( _T( "한줄\n" ) );

    // 출력하고 줄바꿈한다.
    cs.WriteLn( _T( "출력하고 줄바꿈됨" ) );

    // 형식 지정 문자열을 출력한다.
    // https://en.cppreference.com/w/cpp/utility/format/spec
    cs.WriteLn(
        _T( "형식 지정 문자열 (Int = {}, Float = {:.2f}, String = {})" ),
        -1234, 1234.1234, _T( "String" ) );

    // 한글을 출력한다.
    cs.WriteLn( _T( "언어설정 = {}" ), _T( "한국어" ) );

    // 중국어를 출력한다.
    cs.WriteLn( _T( "语言设置 = {}" ), _T( "中文" ) );

    // 일본어를 출력한다.
    cs.WriteLn( _T( "言語設定 = {}" ), _T( "日本語" ) );

    // 이넘을 탐색해서 색상을 출력한다.
    HU_STATIC_ASSERT( ColorTypeInfo::IsValid( -1 ) == false );
    HU_STATIC_ASSERT( ColorTypeInfo::IsValid( 8 ) == false );

    for ( const ColorType color : ColorTypeInfo::values )
        cs.WriteLn( color, _T( "Color = {}" ), ColorTypeInfo::ToStr( color ) );

    // 줄입력을 출력한다.
    //cs.Write( _T( "입력 : " ) );
    //auto input = cs.ReadLn();
    //cs.WriteLn( _T( "입력 = {}" ), input );

    return true;
}
