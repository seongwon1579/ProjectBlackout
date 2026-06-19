// ─── 구현 내역 ───────────────────────
//  - 김민영: 무기 조준/사격 판정 전용 WeaponTrace 콜리전 채널 상수 정의
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

namespace BlackoutCollisionChannels
{
	/** 무기 조준 및 사격 판정에 사용하는 전용 Trace Channel. DefaultEngine.ini의 WeaponTrace와 일치해야 합니다. */
	inline constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel1;
}
