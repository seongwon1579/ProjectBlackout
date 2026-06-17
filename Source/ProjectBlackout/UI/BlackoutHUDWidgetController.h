// ─── 구현 내역 ───────────────────────
//  - 김민영: HUD 데이터 집계·바인딩 컨트롤러(탄약/소모품/유물/착탄/다운/관전 등) 구현
//  - 허혁: 상호작용 프롬프트·부활 UI 분리 및 소모품 픽업 상호작용 연동
// ──────────────────────────────────────

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBlackoutHUDSurrenderVoteStateChangedSignature, bool, bIsActive, int32, YesCount, int32, NoCount, float, EndTimeSeconds);

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

	/**
	 * 관전 상태일 때 현재 ViewTarget의 닉네임을 채워 반환합니다.
	 * 관전 상태가 아니거나 대상이 없으면 false를 반환하고 OutTargetName은 비어 있습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Spectator")
	bool GetSpectatorTargetName(FText& OutTargetName) const;

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

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Surrender")
	FBlackoutHUDSurrenderVoteStateChangedSignature OnSurrenderVoteStateChanged;

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
	void BroadcastWeaponAmmoDisplay(bool bPlaySwapAnimation, FGameplayTag EquippedSlotTagOverride = FGameplayTag());
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

	// ASC의 쿨다운 변경 델리게이트 콜백을 수신하여 UI 슬롯 데이터를 브로드캐스트합니다.
	void HandleConsumableCooldownChanged(FGameplayTag ConsumableTag);

	UFUNCTION()
	void HandleSurrenderVoteStateChanged(bool bIsActive, int32 YesCount, int32 NoCount, float EndTimeSeconds);

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

	// 무기 슬롯 UI는 스왑 애니메이션 재생 위치로 장착 슬롯을 표현하며, 위젯 인스턴스는 캐릭터 교체 시에도
	// 유지된다. 직전에 표시한 슬롯이 주무기인지 추적해, 새 전투 컴포넌트(=캐릭터 교체)에 바인딩될 때
	// 직전이 보조무기였다면 주무기로 강제 전환한다. 위젯 애니메이션 기본 위치는 주무기이므로 true 로 시작.
	bool bLastDisplayedPrimaryEquipped = true;

	/** State 태그 이벤트 핸들. ASC 재바인딩 시 해제용으로 보관합니다. */
	FDelegateHandle DownedTagChangedHandle;
	FDelegateHandle BeingRevivedTagChangedHandle;
	FDelegateHandle DeadTagChangedHandle;
	TWeakObjectPtr<UAbilitySystemComponent> BoundStateTagAbilitySystemComponent;

	// 소모품 쿨다운 변경 델리게이트 핸들 및 바인딩용 ASC 포인터입니다.
	FDelegateHandle ConsumableCooldownChangedHandle;
	TWeakObjectPtr<UAbilitySystemComponent> BoundCooldownAbilitySystemComponent;

	EBlackoutHUDMode CurrentHUDMode = EBlackoutHUDMode::Combat;
};
