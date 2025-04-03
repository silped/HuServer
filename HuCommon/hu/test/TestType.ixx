export module hu.test.TestType;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestType)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.Log;
import hu.Type;

import "hu/log/LogMacro.hpp";
import "hu/serial/SerialMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Struct (TestType)
////////////////////////////////////////////////////////////////////////////////////////////////////

enum SerialInfoType
{
    kSerialMemberInfo = 1,
    kSerialInfo
};

// 직렬화 가능 멤버 객체
struct SerialMemberInfo
{
    String data;

    bool operator==( const SerialMemberInfo& other ) const
    {
        return ( data == other.data );
    }

    void Clear()
    {
        data.clear();
    }

    void Test() const
    {
        HU_LOG_NDEBUG( _T( "Data = {}" ), data );
    }

    HU_SERIAL_MEMBER( SerialMemberInfo, data );
};

// 직렬화 가능 객체
export struct SerialInfo
{
    Int32            id { 0 };
    String           data;
    SerialMemberInfo member;

    bool operator==( const SerialInfo& other ) const
    {
        return ( id == other.id ) && ( data == other.data ) && ( member == other.member );
    }

    void Clear()
    {
        id = 0;
        data.clear();
        member.Clear();
    }

    void Test() const
    {
        HU_LOG_NDEBUG( _T( "Id = {}, Data = {}" ), id, data );
        member.Test();
    }

    // 직렬화 사용을 선언한다.
    HU_SERIAL_MEMBER( SerialInfo, id, data, member );
};

// 직렬화 가능 객체는 비상속, 기본 생성 소멸자의 복사 가능한 플레인 객체를 사용한다.
HU_STATIC_ASSERT( SerialType<SerialMemberInfo> );
HU_STATIC_ASSERT( SerialType<SerialInfo> );
