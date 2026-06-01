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