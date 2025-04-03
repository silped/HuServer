export module hu.test.impl.TestExpression;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestExpression)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <array>;

import hu.log.Log;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (TestExpression)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 생성자 중복 구현 방지 사용 클래스
class CreateObj
{
public:
    // inline 키워드를 사용하면 정적 멤버 변수도 선언부에서 초기화 가능하다.
    inline static const String kConst { _T( "Const" ) };

    // 매개변수 생성자를 먼저 선언한다.
    CreateObj(
        const Int32 val1,
        const Int32 val2
    ) :
        val1_ { val1 },
        val2_ { val2 }
    {
    }

    // 매개변수 생성자를 사용해서 다른 생성자를 선언한다.
    // 매개변수가 1개라 암시적인 형변환을 막기위해 explicit 키워드를 사용한다.
    explicit CreateObj(
        const Int32 val1
    ) :
        CreateObj { val1, 0 }
    {
    }

    CreateObj() :
        CreateObj { 0, 0 }
    {
    }

public:
    bool HasValue(
        const Int32 val1,
        const Int32 val2
    ) const
    {
        return ( val1_ == val1 ) && ( val2_ == val2 );
    }

private:
    Int32 val1_;
    Int32 val2_;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestExpression)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_expression()
{
    // 이진수 입력을 검사한다.
    {
        const UInt8 val1 { 0B11 };
        HU_ASSERT( val1 == 0x03 );

        const UInt8 val2 { 0B1111 };
        HU_ASSERT( val2 == 0x0F );

        const UInt8 val3 { 0B11111111 };
        HU_ASSERT( val3 == 0xFF );
    }

    // 숫자형 자릿수 구분자를 검사한다.
    {
        const Int32 val { 1'000'000 };
        HU_ASSERT( val == 1000000 );
    }

    // 배열 자동 초기화를 검사한다.
    {
        const Int32 vals[ 10 ] { 1, };
        HU_ASSERT( ( vals[ 0 ] == 1 ) && ( vals[ 9 ] == 0 ) );

        const std::array<Int32, 10> vals2 { 1, };
        HU_ASSERT( ( vals2[ 0 ] == 1 ) && ( vals2[ 9 ] == 0 ) );
    }

    // if 문에서 초기화와 조건식 평가를 동시에 한다.
    // 변수의 영역을 한정 시켜 표현력을 좋게한다.
    if ( const Int32 cond { 10 }; cond == 10 )
        HU_LOG_NDEBUG( _T( "cond == 10" ) );

    // switch 문에서 초기화와 조건식 평가를 동시에 한다.
    // 변수의 영역을 한정 시켜 표현력을 좋게한다.
    switch ( const ColorType color { ColorType::kWhite }; color )
    {
        using enum ColorType; // switch 문안에서 네이밍을 생략한다.

    case kBlack:
        HU_LOG_NDEBUG( _T( "color == black" ) );
        break;
    case kWhite:
        HU_LOG_NDEBUG( _T( "color == white" ) );
        break;
    default:
        break;
    }

    // 범위 for 문에서 초기화와 조건식 평가를 동시에 한다.
    // 변수의 영역을 한정 시켜 표현력을 좋게한다.
    Int32 sum { 0 };
    for ( const Int32 vals[] { 10, 20, 30 }; const Int32 val : vals )
        sum += val;
    HU_ASSERT( sum == 60 );

    // 정적 멤버 초기화를 검사한다.
    HU_ASSERT( CreateObj::kConst == _T( "Const" ) );

    // 생성자 사용을 검사한다.
    {
        // 사용자 정의 생성자를 정의 했기 때문에 aggregate 타입이 아니다.
        HU_STATIC_ASSERT( std::is_aggregate_v<CreateObj> == false );

        const CreateObj obj1 { 10, 20 };
        HU_ASSERT( obj1.HasValue( 10, 20 ) );

        const CreateObj obj2 { 10 };
        HU_ASSERT( obj2.HasValue( 10, 0 ) );

        const CreateObj obj3;
        HU_ASSERT( obj3.HasValue( 0, 0 ) );
    }

    // AddressSanitizer가 버퍼 오버 플로우를 감지하는지 검사한다.
    //Int32 overflow_datas[ 10 ] = { 0 };
    //overflow_datas[ 10 ] = 5;

    return true;
}
