#pragma once

import "hu/Macro.hpp";

// 디버그 로그를 기록한다.
#define HU_LOG_DEBUG( category, ... ) hu::Log::Inst().Write( hu::SrcLocation::current(), hu::LogType::kDebug, category, __VA_ARGS__ )

// 디버그 로그를 기록한다.
#define HU_LOG_NDEBUG( ... ) hu::Log::Inst().WriteN( hu::SrcLocation::current(), hu::LogType::kDebug, __VA_ARGS__ )

// 정보 로그를 기록한다.
#define HU_LOG_INFO( category, ... ) hu::Log::Inst().Write( hu::SrcLocation::current(), hu::LogType::kInfo, category, __VA_ARGS__ )

// 정보 로그를 기록한다.
#define HU_LOG_NINFO( ... ) hu::Log::Inst().WriteN( hu::SrcLocation::current(), hu::LogType::kInfo, __VA_ARGS__ )

// 경고 로그를 기록한다.
#define HU_LOG_WARN( category, ... ) hu::Log::Inst().Write( hu::SrcLocation::current(), hu::LogType::kWarn, category, __VA_ARGS__ )

// 경고 로그를 기록한다.
#define HU_LOG_NWARN( ... ) hu::Log::Inst().WriteN( hu::SrcLocation::current(), hu::LogType::kWarn, __VA_ARGS__ )

// 오류 로그를 기록한다.
#define HU_LOG_ERROR( category, ... ) hu::Log::Inst().Write( hu::SrcLocation::current(), hu::LogType::kError, category, __VA_ARGS__ )

// 오류 로그를 기록한다.
#define HU_LOG_NERROR( ... ) hu::Log::Inst().WriteN( hu::SrcLocation::current(), hu::LogType::kError, __VA_ARGS__ )

// 조건을 검사한다.
#define HU_CHECK( cond ) hu::check_impl( cond, _T( #cond ) )

// 조건이 성공 했는지 검사한다.
#define HU_IS_SUCCESS( cond ) hu::check_impl( cond, _T( #cond ) )

// 조건이 실패 했는지 검사한다.
#define HU_IS_FAIL( cond ) hu::check_impl( cond, _T( #cond ) ) == false

// 조건을 검사하고 실패하면 중단 시킨다.
#define HU_ASSERT( cond ) if ( hu::check_impl( cond, _T( #cond ) ) == false ) return false
