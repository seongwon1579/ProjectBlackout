#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UI/BlackoutConsumableTypes.h"
#include "UI/BlackoutHUDTypes.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "UObject/Object.h"
#include "BlackoutHUDWidgetController.generated.h"

class ABOWeaponBase;
class ABlackoutPlayerCharacter;
class ABlackoutPlayerController;
class ABlackoutPlayerState;
class UAbilitySystemComponent;
class UBlackoutAmmoAttributeSet;
class UBlackoutBaseAttributeSet;
class UBlackoutCombatComponent;
class UBlackoutImpactIndicatorComponent;
class UBlackoutPlayerAttributeSet;
class UBOConsumableData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDValueChangedSignature, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlackoutHUDAmmoChangedSignature, int32, ClipAmmo, int32, MaxClipAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDWeaponChangedSignature, ABOWeaponBase*, EquippedWeapon, FGameplayTag, WeaponSlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDAimingChangedSignature, bool, bIsAiming, int32, CrosshairType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlackoutHUDWeaponAmmoDisplayChangedSignature, const FBlackoutWeaponAmmoSlotData&, PrimaryWeaponData, const FBlackoutWeaponAmmoSlotData&, SecondaryWeaponData, bool, bPlaySwapAnimation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDRelicChargesChangedSignature, int32, CurrentCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDConsumablesChangedSignature, int32, BloodRootCount, int32, GulSerumCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutHUDConsumableSlotsChangedSignature, const FBlackoutConsumableSlotData&, BloodRootData, const FBlackoutConsumableSlotData&, GulSerumData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutHUDModeChangedSignature, EBlackoutHUDMode, HUDMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutHUDDownedStateDataChangedSignature, const FBlackoutDownedStateHUDData&, DownedStateHUDData);

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

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Interaction")
	bool GetInteractionPromptData(FBlackoutInteractionPromptData& OutPromptData) const;

	/** 현재 HUD 모드를 반환합니다. 위젯이 초기 모드를 묻거나 누락된 동기화를 보정할 때 사용합니다. */
	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Downed")
	EBlackoutHUDMode GetCurrentHUDMode() const { return CurrentHUDMode; }

	/**
	 * 다운 상태 위젯이 매 틱 폴링할 표시 데이터입니다.
	 * 사망 타이머와 부활 진행률은 시간 기반이므로 위젯이 폴링해 부드럽게 갱신합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Downed")
	bool GetDownedStateHUDData(FBlackoutDownedStateHUDData& OutHUDData) const;

	bool GetRevivePromptData(FBlackoutInteractionPromptData& OutPromptData) const
	{
		return GetInteractionPromptData(OutPromptData);
	}

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDValueChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDValueChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDAmmoChangedSignature OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDWeaponChangedSignature OnEquippedWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDAimingChangedSignature OnAimingChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDWeaponAmmoDisplayChangedSignature OnWeaponAmmoDisplayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDRelicChargesChangedSignature OnRelicChargesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDConsumablesChangedSignature OnConsumablesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD")
	FBlackoutHUDConsumableSlotsChangedSignature OnConsumableSlotsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Downed")
	FBlackoutHUDModeChangedSignature OnHUDModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Downed")
	FBlackoutHUDDownedStateDataChangedSignature OnDownedStateHUDDataChanged;

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
	void BroadcastRelicCharges() const;
	void BroadcastConsumables() const;
	void BroadcastConsumableSlots(int32 BloodRootCount, int32 GulSerumCount) const;
	FBlackoutWeaponAmmoSlotData MakeWeaponAmmoSlotData(ABOWeaponBase* Weapon, FGameplayTag WeaponSlotTag, bool bIsEquipped) const;
	FBlackoutConsumableSlotData MakeConsumableSlotData(UBOConsumableData* ConsumableData) const;
	ABlackoutPlayerCharacter* FindNearbyDownedPlayer(const ABlackoutPlayerCharacter* LocalPlayerCharacter, float ReviveRange) const;
	float GetAttributeValue(const FGameplayAttribute& Attribute) const;
	FGameplayTag GetEquippedWeaponSlotTag() const;
	int32 GetEquippedCrosshairType() const;
	void ProjectTrajectoryPoints(TArray<FBlackoutTrajectoryPointData>& TrajectoryPoints) const;

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleStaminaChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData);
	void HandleRelicChargesChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxRelicChargesChanged(const FOnAttributeChangeData& ChangeData);
	void HandleAmmoChanged(const FOnAttributeChangeData& ChangeData);

	UFUNCTION()
	void HandleConsumablesChanged(int32 BloodRootCount, int32 GulSerumCount);

	/** State.Downed / State.BeingRevived / State.Dead 태그 변경 이벤트 진입점입니다. */
	void HandleDownedRelatedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/** 현재 태그 조합으로 HUD 모드를 평가하고, 변경되었을 때만 브로드캐스트합니다. */
	void RefreshHUDMode();

	/** 다이어그램에 명시된 데이터 빌더입니다. HUD 모드와 잔여 시간/진행률을 계산합니다. */
	FBlackoutDownedStateHUDData BuildDownedStateHUDData(EBlackoutHUDMode InHUDMode) const;

	/** 현재 로컬 플레이어가 가진 State 태그를 읽어 HUD 모드를 결정합니다. */
	EBlackoutHUDMode EvaluateHUDMode() const;

	TWeakObjectPtr<ABlackoutPlayerController> PlayerController;
	TWeakObjectPtr<ABlackoutPlayerState> PlayerState;
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<const UBlackoutBaseAttributeSet> BaseAttributeSet;
	TWeakObjectPtr<const UBlackoutPlayerAttributeSet> PlayerAttributeSet;
	TWeakObjectPtr<const UBlackoutAmmoAttributeSet> AmmoAttributeSet;
	TWeakObjectPtr<UBlackoutCombatComponent> CombatComponent;
	TWeakObjectPtr<UBlackoutImpactIndicatorComponent> ImpactIndicatorComponent;
	TWeakObjectPtr<UBlackoutCombatComponent> BoundCombatComponent;
	TWeakObjectPtr<ABlackoutPlayerState> BoundPlayerState;

	bool bAttributeCallbacksBound = false;

	/** State 태그 이벤트 핸들. ASC 재바인딩 시 해제용으로 보관합니다. */
	FDelegateHandle DownedTagChangedHandle;
	FDelegateHandle BeingRevivedTagChangedHandle;
	FDelegateHandle DeadTagChangedHandle;
	TWeakObjectPtr<UAbilitySystemComponent> BoundStateTagAbilitySystemComponent;

	EBlackoutHUDMode CurrentHUDMode = EBlackoutHUDMode::Combat;
};
