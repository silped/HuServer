#pragma once

import "hu/Macro.hpp";

// 네트워크 디버그 로그를 기록한다.
#define HU_NET_DEBUG( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kDebug, hu::get_net_category_str(), __VA_ARGS__ )

// 네트워크 정보 로그를 기록한다.
#define HU_NET_INFO( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kInfo, hu::get_net_category_str(), __VA_ARGS__ )

// 네트워크 경고 로그를 기록한다.
#define HU_NET_WARN( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kWarn, hu::get_net_category_str(), __VA_ARGS__ )

// 네트워크 오류 로그를 기록한다.
#define HU_NET_ERROR( ... ) hu::Log::Inst().Write( SrcLocation::current(), hu::LogType::kError, hu::get_net_category_str(), __VA_ARGS__ )
