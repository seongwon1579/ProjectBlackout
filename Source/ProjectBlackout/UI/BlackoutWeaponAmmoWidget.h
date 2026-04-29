#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "BlackoutWeaponAmmoWidget.generated.h"

class UBlackoutWeaponAmmoSlotWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutWeaponAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Weapon")
	void SetWeaponAmmoData(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData GetPrimaryWeaponData() const { return PrimarySlotData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData GetSecondaryWeaponData() const { return SecondarySlotData; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Weapon")
	TObjectPtr<UBlackoutWeaponAmmoSlotWidget> PrimaryWeaponSlotWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Weapon")
	TObjectPtr<UBlackoutWeaponAmmoSlotWidget> SecondaryWeaponSlotWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData PrimarySlotData;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData SecondarySlotData;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Ammo Display Changed"), Category = "Blackout|HUD|Weapon")
	void ReceiveWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Swap Animation Requested"), Category = "Blackout|HUD|Weapon")
	void ReceiveWeaponSwapAnimationRequested(bool bIsPrimaryEquipped);
};
