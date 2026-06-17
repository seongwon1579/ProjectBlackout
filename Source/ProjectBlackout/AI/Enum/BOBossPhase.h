// ─── 구현 내역 ───────────────────────
//  - 조성원: 보스 페이즈 단계 enum 정의 (None/Phase1~3)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BOBossPhase.generated.h"

UENUM(BlueprintType)
enum class EBOBossPhase : uint8
{
	None    UMETA(DisplayName = "None"),
	Phase1  UMETA(DisplayName = "Phase 1"),
	Phase2  UMETA(DisplayName = "Phase 2"),
	Phase3  UMETA(DisplayName = "Phase 3"),
};