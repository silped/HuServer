export module hu.pattern.ISingleton;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ISingleton)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.pattern.INoCopy;


namespace hu {

// 싱글톤 기본 구현체
export template <typename T>
class ISingleton : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ISingleton)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    ISingleton() = default;
    ~ISingleton() = default;

    ISingleton( ISingleton&& ) = delete;
    ISingleton& operator=( ISingleton&& ) = delete;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ISingleton)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    using Instance = T;

    // 인스턴스를 얻는다.
    static Instance& Inst()
    {
        static Instance inst;
        return inst;
    }
};

} // hu
