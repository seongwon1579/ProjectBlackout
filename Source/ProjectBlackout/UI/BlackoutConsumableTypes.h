#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlackoutConsumableTypes.generated.h"

class UBOConsumableData;
class UTexture2D;

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutConsumableSlotData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	TObjectPtr<UBOConsumableData> ConsumableData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	FGameplayTag ConsumableTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	int32 MaxCount = 0;

	// 현재 남은 소모품 쿨다운 시간 (초)
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	float CooldownRemaining = 0.0f;

	// 소모품의 총 쿨다운 지속시간 (초)
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	float CooldownDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	bool bIsValid = false;
};
