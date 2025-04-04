export module hu.container.MapBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (MapBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <functional>;
import <unordered_map>;

import hu.Type;


namespace hu {

// 맵 기본형을 구현한 클래스
export template <typename KEY_T, typename VAL_T, typename IMPL_T = std::unordered_map<KEY_T, VAL_T>>
class MapBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MapBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    MapBase() = default;
    virtual ~MapBase() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MapBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 맵 순회 처리에 사용할 함수로 false를 리턴하면 순회를 중단한다.
    using LoopFunction = std::function<bool( const KEY_T&, const VAL_T& )>;

public:
    // 요소 개수를 얻는다.
    Size GetCount() const
    {
        return impl_.size();
    }

    // 비어 있는지 검사한다.
    bool IsEmpty() const
    {
        return impl_.empty();
    }

    // 요소를 추가한다.
    bool Add(
        const KEY_T& key,
        const VAL_T& val
    )
    {
        if ( impl_.contains( key ) )
            return false;

        impl_[ key ] = val;
        return true;
    }

    // 요소를 제거한다.
    bool Delete(
        const KEY_T& key
    )
    {
        auto it = impl_.find( key );
        if ( it == impl_.end() )
            return false;

        impl_.erase( it );
        return true;
    }

    // 읽기 전용 값을 찾는다.
    const VAL_T* Find(
        const KEY_T& key
    ) const
    {
        auto it = impl_.find( key );
        if ( it != impl_.cend() )
            return &( it->second );

        return nullptr;
    }

    // 쓰기 가능한 값을 찾는다.
    VAL_T* FindW(
        const KEY_T& key
    )
    {
        auto it = impl_.find( key );
        if ( it != impl_.end() )
            return &( it->second );

        return nullptr;
    }

    // 요소를 순회 처리한다.
    bool Loop(
        const LoopFunction& func
    ) const
    {
        for ( const auto& [ key, val ] : impl_ )
        {
            if ( func( key, val ) == false )
                return false;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (MapBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    IMPL_T impl_;
};

} // hu
