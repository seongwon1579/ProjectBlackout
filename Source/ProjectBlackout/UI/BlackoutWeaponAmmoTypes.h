#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlackoutWeaponAmmoTypes.generated.h"

class ABOWeaponBase;
class UTexture2D;

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutWeaponAmmoSlotData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	TObjectPtr<ABOWeaponBase> Weapon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	TObjectPtr<UTexture2D> WeaponIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	FGameplayTag WeaponSlotTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	int32 ReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	int32 TotalAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	bool bIsEquipped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	bool bUsesAmmo = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	bool bIsValid = false;
};
