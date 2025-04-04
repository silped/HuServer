#pragma once

import <yas/object.hpp>;

// 직렬화 사용을 선언한다.
#define HU_SERIAL_MEMBER( type, ... ) \
    static constexpr auto kType { k##type }; \
    YAS_DEFINE_STRUCT_SERIALIZE( #type, __VA_ARGS__ )
