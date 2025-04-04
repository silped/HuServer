export module hu.Type;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Type)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <array>;
import <chrono>;
import <cstdint>;
import <source_location>;
import <string_view>;
import <string>;
import <type_traits>;
import <vector>;

import "hu/Macro.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (Type)
////////////////////////////////////////////////////////////////////////////////////////////////////

export using Int8   = std::int8_t;
export using UInt8  = std::uint8_t;
export using Int16  = std::int16_t;
export using UInt16 = std::uint16_t;
export using Int32  = std::int32_t;
export using UInt32 = std::uint32_t;
export using Int64  = std::int64_t;
export using UInt64 = std::uint64_t;
export using Size   = size_t;

export template <typename T, Size N>
using Array = std::array<T, N>;

export using Char       = wchar_t;
export using String     = std::wstring;
export using StringView = std::wstring_view;

export using Byte   = std::uint8_t;
export using Buffer = std::vector<Byte>;

export using SrcLocation = std::source_location;

export using namespace std::chrono_literals;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Concept (Type)
////////////////////////////////////////////////////////////////////////////////////////////////////

export template <typename T>
concept IntType = std::is_integral_v<T>;

// 열거형 타입
export template <typename T>
concept EnumType = std::is_enum_v<T>;

// 복사 가능 타입
export template <typename T>
concept CopyableType = std::is_trivially_copyable_v<T>;

// POD 타입
export template <typename T>
concept PodType = std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

// 직렬화 타입
export template <typename T>
concept SerialType = std::is_aggregate_v<T>;

// 포인터 타입
export template <typename T>
concept PointerType = std::is_pointer_v<T>;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Enum (Type)
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace util {

// 포인터를 삭제한다.
export template <PointerType T>
constexpr void delete_ptr(
    T& ptr
)
{
    if ( ptr )
    {
        delete ptr;
        ptr = nullptr;
    }
}

// 열거형을 숫자로 변환한다.
export template <EnumType T>
constexpr auto enum_to_num(
    const T e
)
{
    return static_cast<std::underlying_type_t<T>>( e );
}

} // hu::util

// 색상 종류
HU_CREATE_ENUM( ColorType, Black, Red, Green, Yellow, Blue, Magenta, Cyan, White );

} // hu
