export module hu.db.local.LocalDBConfig;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LocalDBConfig)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.local.LocalDBType;
import hu.Type;

import "hu/Macro.hpp";


namespace hu {

// 로컬 디비 환경 설정
export struct LocalDBConfigInfo final
{
    // 디비 저장 디렉터리
    String dir { _T( "_db" ) };

    // 디비 이름
    String name { _T( "HuTestDB" ) };

    // 디비 구현 종류
    LocalDBImplType impl { LocalDBImplType::kRocksDB };

    // 설정이 유효한지 검사한다.
    bool IsValid() const
    {
        if ( name.empty() )
            return false;

        return true;
    }
};

} // hu
