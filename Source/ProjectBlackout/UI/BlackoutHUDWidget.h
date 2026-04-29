#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "BlackoutHUDWidget.generated.h"

class ABOWeaponBase;
class UBlackoutHUDWidgetController;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	virtual void SetWidgetController(UBlackoutHUDWidgetController* InWidgetController);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidgetController* GetWidgetController() const { return WidgetController; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidgetController> WidgetController;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Widget Controller Set"), Category = "Blackout|HUD")
	void ReceiveWidgetControllerSet();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Health Changed"), Category = "Blackout|HUD")
	void ReceiveHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Stamina Changed"), Category = "Blackout|HUD")
	void ReceiveStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Ammo Changed"), Category = "Blackout|HUD")
	void ReceiveAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Equipped Weapon Changed"), Category = "Blackout|HUD")
	void ReceiveEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

private:
	void UnbindWidgetControllerCallbacks();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void HandleAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);
};
