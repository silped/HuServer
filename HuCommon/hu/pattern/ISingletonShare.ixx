export module hu.pattern.ISingletonShare;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ISingletonShare)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;

import hu.pattern.INoCopy;


namespace hu {

// 싱글톤 객체 공유형 구현체
export template <typename T>
class ISingletonShare : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ISingletonShare)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    ISingletonShare() = default;
    ~ISingletonShare() = default;

    ISingletonShare( ISingletonShare&& ) = delete;
    ISingletonShare& operator=( ISingletonShare&& ) = delete;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ISingletonShare)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    using Instance = std::shared_ptr<T>;

    // 인스턴스를 얻는다.
    static Instance Inst()
    {
        const static Instance inst { std::make_shared<T>() };
        return inst;
    }
};

} // hu
