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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorDefaultColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorMismatchColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorOccludedColor = FLinearColor(1.0f, 0.7f, 0.0f, 1.0f);

	/**
	 * 탄퍼짐이 최대일 때 인디케이터 위젯에 적용되는 RenderScale 배율.
	 * 1.0 = 원본 크기, 값이 클수록 최대 탄퍼짐 시 인디케이터가 더 커집니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD", meta = (ClampMin = 1.0f))
	float MaxSpreadIndicatorScale = 3.0f;

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
	void ReceiveAimingChanged(bool bIsAiming, int32 CrosshairType);

	/**
	 * 매 틱 호출됩니다. 외부 크로스헤어 에셋에 현재 탄퍼짐을 전달할 때 사용하세요.
	 * @param NormalizedSpread 0(기본 탄퍼짐) ~ 1(최대 탄퍼짐)
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Spread Updated"), Category = "Blackout|HUD")
	void ReceiveSpreadUpdated(float NormalizedSpread);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Ammo Display Changed"), Category = "Blackout|HUD")
	void ReceiveWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);

private:
	void UnbindWidgetControllerCallbacks();
	void UpdateImpactIndicator(const FBlackoutImpactIndicatorData& ImpactIndicatorData) const;
	void ApplyImpactIndicatorColor(const FLinearColor& IndicatorColor) const;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void HandleAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

	UFUNCTION()
	void HandleAimingChanged(bool bIsAiming, int32 CrosshairType);

	UFUNCTION()
	void HandleWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);
};
