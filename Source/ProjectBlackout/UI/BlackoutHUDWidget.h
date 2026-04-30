#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "UI/BlackoutHUDTypes.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "BlackoutHUDWidget.generated.h"

class ABOWeaponBase;
class UBlackoutHUDWidgetController;
class UBlackoutValueBarWidget;
class UBlackoutWeaponAmmoWidget;
class UWidget;

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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutValueBarWidget> HealthBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutValueBarWidget> StaminaBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutWeaponAmmoWidget> AmmoWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UWidget> ImpactIndicatorWidget;

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

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Aiming Changed"), Category = "Blackout|HUD")
	void ReceiveAimingChanged(bool bIsAiming);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Ammo Display Changed"), Category = "Blackout|HUD")
	void ReceiveWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);

private:
	void UnbindWidgetControllerCallbacks();
	void UpdateImpactIndicator(const FBlackoutImpactIndicatorData& ImpactIndicatorData) const;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void HandleAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

	UFUNCTION()
	void HandleAimingChanged(bool bIsAiming);

	UFUNCTION()
	void HandleWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);
};
