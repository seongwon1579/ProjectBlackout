// ─── 구현 내역 ───────────────────────
//  - 김민영: HUD 무기 탄약 슬롯 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "BlackoutWeaponAmmoSlotWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutWeaponAmmoSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Weapon")
	void SetWeaponAmmoData(const FBlackoutWeaponAmmoSlotData& NewSlotData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData GetWeaponAmmoData() const { return SlotData; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Weapon")
	FBlackoutWeaponAmmoSlotData SlotData;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Weapon")
	TObjectPtr<UImage> WeaponImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Weapon")
	TObjectPtr<UTextBlock> CurrentAmmoText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Weapon")
	TObjectPtr<UTextBlock> ReserveAmmoText;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Ammo Data Changed"), Category = "Blackout|HUD|Weapon")
	void ReceiveWeaponAmmoDataChanged(const FBlackoutWeaponAmmoSlotData& NewSlotData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Equipped State Changed"), Category = "Blackout|HUD|Weapon")
	void ReceiveEquippedStateChanged(bool bIsEquipped);
};
