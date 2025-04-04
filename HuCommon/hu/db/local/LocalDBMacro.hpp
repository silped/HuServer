#pragma once

import "hu/Macro.hpp";

// 로컬 디비 디버그 로그를 기록한다.
#define HU_LOCALDB_DEBUG( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kDebug, hu::get_localdb_category_str(), __VA_ARGS__ )

// 로컬 디비 정보 로그를 기록한다.
#define HU_LOCALDB_INFO( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kInfo, hu::get_localdb_category_str(), __VA_ARGS__ )

// 로컬 디비 경고 로그를 기록한다.
#define HU_LOCALDB_WARN( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kWarn, hu::get_localdb_category_str(), __VA_ARGS__ )

// 로컬 디비 오류 로그를 기록한다.
#define HU_LOCALDB_ERROR( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kError, hu::get_localdb_category_str(), __VA_ARGS__ )
