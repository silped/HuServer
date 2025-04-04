export module hu.serial.impl.YasSerializer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (YasSerializer)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <yas/object.hpp>;
import <yas/serialize.hpp>;
import <yas/std_types.hpp>;
 
import hu.pattern.INoCopy;
import hu.Type;


namespace hu {

// YAS(https://github.com/niXman/yas)를 사용한 직렬화 구현체
export template <auto kOption>
class YasSerializer final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (YasSerializer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 객체를 버퍼에 쓰기 위한 크기를 얻는다.
    static Size GetWriteSize(
        const auto& obj
    )
    {
        try
        {
            return yas::saved_size<kOption>( obj );
        }
        catch ( ... )
        {
            return 0;
        }
    }

    // 객체를 버퍼에 쓴다.
    static bool Write(
        const auto& obj,
        Buffer&     buffer
    )
    {
        try
        {
            yas::save<kFlag>( yas::vector_ostream<Byte>( buffer ), obj );
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    }

    // 버퍼에서 객체를 읽는다.
    static bool Read(
        const Buffer& buffer,
        auto&         obj
    )
    {
        try
        {
            yas::load<kFlag>( buffer, obj );
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (YasSerializer)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    static constexpr Size kFlag = ( kOption | yas::mem );
};

export using YasBinSerializer  = YasSerializer<yas::binary>;
export using YasJsonSerializer = YasSerializer<yas::json>;

} // hu
