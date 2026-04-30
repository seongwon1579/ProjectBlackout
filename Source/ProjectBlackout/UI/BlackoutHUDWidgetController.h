#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UI/BlackoutHUDTypes.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "UObject/Object.h"
#include "BlackoutHUDWidgetController.generated.h"

class ABOWeaponBase;
class ABlackoutPlayerController;
class ABlackoutPlayerState;
class UAbilitySystemComponent;
class UBlackoutAmmoAttributeSet;
class UBlackoutBaseAttributeSet;
class UBlackoutCombatComponent;
class UBlackoutPlayerAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDValueChangedSignature, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlackoutHUDAmmoChangedSignature, int32, ClipAmmo, int32, MaxClipAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDWeaponChangedSignature, ABOWeaponBase*, EquippedWeapon, FGameplayTag, WeaponSlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutHUDBoolChangedSignature, bool, bNewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlackoutHUDWeaponAmmoDisplayChangedSignature, const FBlackoutWeaponAmmoSlotData&, PrimaryWeaponData, const FBlackoutWeaponAmmoSlotData&, SecondaryWeaponData, bool, bPlaySwapAnimation);

UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBlackoutHUDWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	bool Initialize(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	void BindCallbacksToDependencies();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	void BroadcastInitialValues();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	bool GetImpactIndicatorData(FBlackoutImpactIndicatorData& OutIndicatorData) const;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDValueChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDValueChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDAmmoChangedSignature OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDWeaponChangedSignature OnEquippedWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDBoolChangedSignature OnAimingChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDWeaponAmmoDisplayChangedSignature OnWeaponAmmoDisplayChanged;

protected:
	UFUNCTION()
	void HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

	UFUNCTION()
	void HandleAimingChanged(bool bIsAiming);

private:
	bool ResolveDependencies(APlayerController* InPlayerController);
	void BroadcastHealth() const;
	void BroadcastStamina() const;
	void BroadcastAmmo() const;
	void BroadcastEquippedWeapon() const;
	void BroadcastAiming() const;
	void BroadcastWeaponAmmoDisplay(bool bPlaySwapAnimation) const;
	FBlackoutWeaponAmmoSlotData MakeWeaponAmmoSlotData(ABOWeaponBase* Weapon, FGameplayTag WeaponSlotTag, bool bIsEquipped) const;
	float GetAttributeValue(const FGameplayAttribute& Attribute) const;
	FGameplayTag GetEquippedWeaponSlotTag() const;

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleStaminaChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData);
	void HandleAmmoChanged(const FOnAttributeChangeData& ChangeData);

	TWeakObjectPtr<ABlackoutPlayerController> PlayerController;
	TWeakObjectPtr<ABlackoutPlayerState> PlayerState;
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<const UBlackoutBaseAttributeSet> BaseAttributeSet;
	TWeakObjectPtr<const UBlackoutPlayerAttributeSet> PlayerAttributeSet;
	TWeakObjectPtr<const UBlackoutAmmoAttributeSet> AmmoAttributeSet;
	TWeakObjectPtr<UBlackoutCombatComponent> CombatComponent;

	bool bCallbacksBound = false;
};
