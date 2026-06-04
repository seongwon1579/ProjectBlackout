#pragma once

#include "CoreMinimal.h"
#include "BlackoutStencil.generated.h"

UENUM(BlueprintType)
enum class EBlackoutStencil :uint8
{
	None = 0 UMETA(DisplayName = "None"), // 외곽선 없음
	Enemy = 1 UMETA(DisplayName= "Enemy"), // 적 
	Player = 2  UMETA(DisplayName= "Player"), // 아군
	// Focused =3  UMETA(DisplayName = "Focused") 
};
