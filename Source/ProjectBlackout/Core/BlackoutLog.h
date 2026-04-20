#pragma once

#include "CoreMinimal.h"
#include "BlackoutLogCategories.h"

/**
 * Project Blackout 전용 로그 및 디버그 매크로.
 *
 * BO_LOG   (Category, Verbosity, Format, ...)  — 함수명·라인 포함 UE_LOG
 * BO_SCREEN(Category, Verbosity, Format, ...)  — 위 + GEngine 화면 출력 (색상: Log=Cyan / Warning=Yellow / Error=Red)
 * BO_CHECK (Condition, Format, ...)            — 조건 실패 시 Error 로그 + ensure
 *
 * 카테고리별 단축 매크로:
 *   BO_LOG_CORE  / BO_SCREEN_CORE   → LogBlackout
 *   BO_LOG_GAS   / BO_SCREEN_GAS    → LogBlackoutGAS
 *   BO_LOG_AI    / BO_SCREEN_AI     → LogBlackoutAI
 *   BO_LOG_NET   / BO_SCREEN_NET    → LogBlackoutNet
 *   BO_LOG_POOL  / BO_SCREEN_POOL   → LogBlackoutPool
 */

// 함수명 + 라인 정보
#define BO_LOG_CALLINFO (FString(__FUNCTION__) + TEXT(" (") + FString::FromInt(__LINE__) + TEXT(")"))

// ── 기본 매크로 ───────────────────────────────────────────────────────────────

#define BO_LOG(Category, Verbosity, Format, ...) \
	UE_LOG(Category, Verbosity, TEXT("%s | " Format), *BO_LOG_CALLINFO, ##__VA_ARGS__)

#define BO_SCREEN(Category, Verbosity, Format, ...) \
{ \
	BO_LOG(Category, Verbosity, Format, ##__VA_ARGS__); \
	if (GEngine) \
	{ \
		FColor _Color = FColor::Cyan; \
		const FString _VStr = TEXT(#Verbosity); \
		if      (_VStr == TEXT("Warning")) _Color = FColor::Yellow; \
		else if (_VStr == TEXT("Error"))   _Color = FColor::Red; \
		GEngine->AddOnScreenDebugMessage(-1, 5.f, _Color, \
			FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
	} \
}

// 조건 체크 — 실패 시 LogBlackout Error + ensure
#define BO_CHECK(Condition, Format, ...) \
{ \
	if (!(Condition)) \
	{ \
		BO_SCREEN(LogBlackout, Error, "CHECK FAILED: %s | " Format, TEXT(#Condition), ##__VA_ARGS__); \
		ensureMsgf(false, TEXT("CHECK FAILED: %s | " Format), TEXT(#Condition), ##__VA_ARGS__); \
	} \
}

// ── 카테고리별 단축 매크로 ────────────────────────────────────────────────────

#define BO_LOG_CORE(Verbosity, Format, ...)  BO_LOG(LogBlackout,     Verbosity, Format, ##__VA_ARGS__)
#define BO_LOG_GAS(Verbosity, Format, ...)   BO_LOG(LogBlackoutGAS,  Verbosity, Format, ##__VA_ARGS__)
#define BO_LOG_AI(Verbosity, Format, ...)    BO_LOG(LogBlackoutAI,   Verbosity, Format, ##__VA_ARGS__)
#define BO_LOG_NET(Verbosity, Format, ...)   BO_LOG(LogBlackoutNet,  Verbosity, Format, ##__VA_ARGS__)
#define BO_LOG_POOL(Verbosity, Format, ...)  BO_LOG(LogBlackoutPool, Verbosity, Format, ##__VA_ARGS__)

#define BO_SCREEN_CORE(Verbosity, Format, ...)  BO_SCREEN(LogBlackout,     Verbosity, Format, ##__VA_ARGS__)
#define BO_SCREEN_GAS(Verbosity, Format, ...)   BO_SCREEN(LogBlackoutGAS,  Verbosity, Format, ##__VA_ARGS__)
#define BO_SCREEN_AI(Verbosity, Format, ...)    BO_SCREEN(LogBlackoutAI,   Verbosity, Format, ##__VA_ARGS__)
#define BO_SCREEN_NET(Verbosity, Format, ...)   BO_SCREEN(LogBlackoutNet,  Verbosity, Format, ##__VA_ARGS__)
#define BO_SCREEN_POOL(Verbosity, Format, ...)  BO_SCREEN(LogBlackoutPool, Verbosity, Format, ##__VA_ARGS__)
