export module hu.pattern.INoCopy;

namespace hu {

// 복사 방지 구현체
export class INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (INoCopy)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    INoCopy() = default;
    ~INoCopy() = default;

    INoCopy( const INoCopy& ) = delete;
    INoCopy& operator=( const INoCopy& ) = delete;
};

} // hu
