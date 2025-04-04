#pragma once

import <boost/preprocessor.hpp>;

#define _T( s ) L##s

// 정적으로 조건을 검사한다.
#define HU_STATIC_ASSERT( cond ) static_assert( cond, #cond )

// 가변인자를 반복해서 개별 처리한다.
#define HU_EXPAND( v ) v
#define HU_FOR_EACH_1( what, v, ... ) what( v )
#define HU_FOR_EACH_2( what, v, ... ) what( v ), HU_EXPAND( HU_FOR_EACH_1( what, __VA_ARGS__ ) )
#define HU_FOR_EACH_3( what, v, ... ) what( v ), HU_EXPAND( HU_FOR_EACH_2( what, __VA_ARGS__ ) )
#define HU_FOR_EACH_4( what, v, ... ) what( v ), HU_EXPAND( HU_FOR_EACH_3( what, __VA_ARGS__ ) )
#define HU_FOR_EACH_5( what, v, ... ) what( v ), HU_EXPAND( HU_FOR_EACH_4( what, __VA_ARGS__ ) )
#define HU_GET_MACRO( _1, _2, _3, _4, _5, name, ... ) name
#define HU_FOR_EACH( action, ... ) HU_EXPAND( HU_GET_MACRO( __VA_ARGS__, HU_FOR_EACH_5, HU_FOR_EACH_4, HU_FOR_EACH_3, HU_FOR_EACH_2, HU_FOR_EACH_1 )( action, __VA_ARGS__ ) )

// 가변인자 개별 처리 매크로는 아래 처럼 사용한다.
//#define HU_ENUM_TO_CONST( v ) k##v // 개별 인자에 적용할 매크로
//values { HU_FOR_EACH( HU_ENUM_TO_CONST, __VA_ARGS__ ) }; // 전체 인자에 적용할 매크로

#define HU_ENUM_TO_CONST( r, data, elem ) k##elem
#define HU_ENUM_TO_STR( r, data, elem ) _T( #elem )

#define HU_CREATE_ENUM( name, ... ) \
export enum class name { BOOST_PP_SEQ_ENUM( BOOST_PP_SEQ_TRANSFORM( HU_ENUM_TO_CONST, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) ) ) }; \
\
export struct name##Info \
{ \
    using enum name; \
    static constexpr auto kCount { BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ) }; \
    inline static constexpr Array<name, kCount> values { BOOST_PP_SEQ_ENUM( BOOST_PP_SEQ_TRANSFORM( HU_ENUM_TO_CONST, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) ) ) }; \
\
    static constexpr name kFirst { values[ 0 ] }; \
    static constexpr name kLast { values[ kCount - 1 ] }; \
    static constexpr auto kFirstNum { hu::util::enum_to_num( kFirst ) }; \
    static constexpr auto kLastNum { hu::util::enum_to_num( kLast ) }; \
\
    static constexpr bool IsValid( const auto num ) { return ( num >= kFirstNum ) && ( num <= kLastNum ); } \
    static constexpr name ToEnum( const auto num ) { if ( IsValid( num ) ) return static_cast<name>( num ); return kFirst; } \
    static constexpr auto ToNum( const name val ) { return hu::util::enum_to_num( val ); } \
    static const String& ToStr( const name val ) { \
        static const Array<const String, kCount> strs = { BOOST_PP_SEQ_ENUM( BOOST_PP_SEQ_TRANSFORM( HU_ENUM_TO_STR, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) ) ) }; \
        return strs[ ToNum( val ) ]; \
    } \
}
