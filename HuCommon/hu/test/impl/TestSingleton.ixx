export module hu.test.impl.TestSingleton;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestSingleton)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.pattern.ISingletonShare;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Struct (TestSingleton)
////////////////////////////////////////////////////////////////////////////////////////////////////
 
// 타입이 아닌 템플릿 인자는 auto 사용 가능하다.
template <auto kId>
struct BaseInfo
{
    BaseInfo()
    {
        HU_LOG_NINFO( _T( "Create (Id = {})" ), kId );
    }

    ~BaseInfo()
    {
        HU_LOG_NINFO( _T( "Destroy (Id = {})" ), kId );
    }

    void Test()
    {
        HU_LOG_NINFO( _T( "Test (Id = {})" ), kId );
    }
};

struct Info1 : public BaseInfo<1>, public ISingletonShare<Info1>
{
};

struct Info2 : public BaseInfo<2>, public ISingletonShare<Info2>
{
    Info2() = default;

    ~Info2()
    {
        info1->Test();
    }

    Info1::Instance info1 { Info1::Inst() };
};

struct Info3 : public BaseInfo<3>, public ISingletonShare<Info3>
{
    Info3() = default;

    ~Info3()
    {
        info2->Test();
    }

    Info2::Instance info2 { Info2::Inst() };
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestSingleton)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_singleton()
{
    // 싱글톤 객체간 참조 순서에 따라 자동으로 3 -> 2 -> 1 순서로 소멸한다.
    Info3::Inst()->Test();

    return true;
}
