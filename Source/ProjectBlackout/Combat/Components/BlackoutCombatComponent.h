#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BlackoutTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "BlackoutCombatComponent.generated.h"

class ABOWeaponBase;
class ABOFirearm;
class ABOMeleeWeapon;
class AController;
class UBOCharacterData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutEquippedWeaponChangedSignature, ABOWeaponBase*, EquippedWeapon, FGameplayTag, WeaponSlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutAimingChangedSignature, bool, bIsAiming);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBlackoutCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void InitializeLoadoutFromCharacterData(const UBOCharacterData* CharacterData);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EquipPrimary();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EquipSecondary();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void SwapWeapon();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void HandlePrimaryActionPressed();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void HandlePrimaryActionReleased();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void StartAim();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void StopAim();

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	bool CanAim() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void TryReload();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void PerformMeleeHit(const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void CommitPendingWeaponSwap();

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	bool IsWeaponSwapInProgress() const { return bIsWeaponSwapInProgress; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void BeginMeleeWeaponAttachmentOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EndMeleeWeaponAttachmentOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void BeginEquippedWeaponHolsterOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EndEquippedWeaponHolsterOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void BeginEquippedWeaponHiddenOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EndEquippedWeaponHiddenOverride();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOFirearm* GetPrimaryWeapon() const { return PrimaryWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOFirearm* GetSecondaryWeapon() const { return SecondaryWeapon; }

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Combat|Events")
	FBlackoutEquippedWeaponChangedSignature OnEquippedWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Combat|Events")
	FBlackoutAimingChangedSignature OnAimingChanged;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOFirearm* GetEquippedFirearm() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOMeleeWeapon* GetMeleeWeapon() const { return MeleeWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayTag GetEquippedWeaponSlotTag() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	/** 발사 1회 시 호출. 탄퍼짐을 누적하고 반동을 카메라에 적용합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void OnShotFired();

	/**
	 * 기본 발사 방향에 현재 탄퍼짐을 적용한 방향을 반환합니다.
	 * 탄퍼짐 콘 안에서 무작위 편향된 방향이 반환됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FVector GetSpreadDeviatedDirection(const FVector& BaseDirection) const;

	/** 현재 탄퍼짐 각도를 반환합니다 (도). */
	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	float GetCurrentSpreadDegrees() const { return CurrentSpreadDegrees; }

	/** 현재 탄퍼짐을 0(기본)~1(최대) 범위로 정규화하여 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	float GetNormalizedSpread() const;

	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(ABOWeaponBase* NewWeapon);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void Server_BeginWeaponSwap(FGameplayTag TargetWeaponSlotTag);

	UFUNCTION(Server, Reliable)
	void Server_CommitPendingWeaponSwap(FGameplayTag TargetWeaponSlotTag);

	UFUNCTION(Server, Reliable)
	void Server_CancelPendingWeaponSwap();

	void HandleWeaponSwapMontageEnded(bool bInterrupted);

	// 근접 공격 윈도우 
	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void BeginMeleeAttackWindow(const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void UpdateMeleeAttackWindow();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EndMeleeAttackWindow();
	
protected:
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_LoadoutWeapon();

	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION()
	void OnRep_MeleeWeaponAttachmentOverride();

	UFUNCTION()
	void OnRep_EquippedWeaponHolsterOverride();

	UFUNCTION()
	void OnRep_EquippedWeaponHiddenOverride();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyEquippedWeaponHolsterOverride(bool bNewHolsterOverride);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyEquippedWeaponHiddenOverride(bool bNewHiddenOverride);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly, Category = "Blackout|Combat|Weapon")
	TObjectPtr<ABOWeaponBase> EquippedWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat|Weapon")
	TObjectPtr<ABOFirearm> PrimaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat|Weapon")
	TObjectPtr<ABOFirearm> SecondaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat|Weapon")
	TObjectPtr<ABOMeleeWeapon> MeleeWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "Blackout|Combat|State")
	bool bIsAiming = false;

	UPROPERTY(ReplicatedUsing = OnRep_MeleeWeaponAttachmentOverride, BlueprintReadOnly, Category = "Blackout|Combat|State")
	bool bMeleeWeaponAttachmentOverride = false;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeaponHolsterOverride, BlueprintReadOnly, Category = "Blackout|Combat|State")
	bool bEquippedWeaponHolsterOverride = false;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeaponHiddenOverride, BlueprintReadOnly, Category = "Blackout|Combat|State")
	bool bEquippedWeaponHiddenOverride = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat|Settings")
	FName EquippedWeaponSocketName = TEXT("WeaponSocket");

	/** 반동이 목표 값에 도달하는 보간 속도. 클수록 즉각적. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat|Recoil", meta = (ClampMin = 1.f))
	float RecoilInterpSpeed = 15.0f;

	/** 반동 종료 후 카메라가 되돌아오는 보간 속도. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat|Recoil", meta = (ClampMin = 1.f))
	float RecoilRecoveryInterpSpeed = 8.0f;

private:
	ABOWeaponBase* SpawnWeaponActor(TSubclassOf<ABOWeaponBase> WeaponClass);
	void RefreshWeaponAttachments() const;
	void ApplyInitialAmmoLoadout() const;
	bool CanStartAim() const;
	float GetEquippedClipAmmo() const;
	void ApplyAimingState(bool bNewAiming);
	ABOWeaponBase* ResolveWeaponBySlotTag(FGameplayTag WeaponSlotTag) const;
	bool BeginWeaponSwapInternal(FGameplayTag TargetWeaponSlotTag);
	EBlackoutAbilityInputID ResolvePrimaryActionInputID() const;
	void StartAutomaticFire();
	void StopAutomaticFire();
	void HandleAutomaticFireTick();
	void ReleaseActivePrimaryAction();
	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID) const;
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID) const;

	
	// 현재 열려 있는 근접 공격창에서 사용할 데미지 스펙
	FGameplayEffectSpecHandle ActiveMeleeDamageSpecHandle;

	// 같은 공격창 안에서 동일 히트박스 중복 피격을 막기 위한 집합
	TSet<TWeakObjectPtr<UPrimitiveComponent>> ProcessedMeleeHitComponents;

	// 같은 공격창 안에서 동일 액터 중복 피격을 막기 위한 집합
	TSet<TWeakObjectPtr<AActor>> ProcessedMeleeHitActors;

	// 현재 근접 공격창이 열려 있는지 여부
	bool bMeleeAttackWindowActive = false;
	
	UPROPERTY(Transient)
	EBlackoutAbilityInputID ActivePrimaryActionInputID = EBlackoutAbilityInputID::None;

	UPROPERTY(Transient)
	FGameplayTag PendingWeaponSwapSlotTag;

	UPROPERTY(Transient)
	bool bIsWeaponSwapInProgress = false;

	FTimerHandle AutomaticFireTimerHandle;

	void AccumulateSpread();
	void ApplyRecoil();
	void TickSpreadRecovery();
	void TickRecoil(float DeltaTime);
	void ResetSpread();
	float GetRecoilPitchDisplacement(const AController& Controller) const;

	float CurrentSpreadDegrees = 0.0f;
	FTimerHandle SpreadRecoveryTimerHandle;

	float PendingRecoilPitch = 0.0f;
	float PendingRecoilYaw = 0.0f;
	float RecoilBaselinePitch = 0.0f;
	float AccumulatedRecoilPitch = 0.0f;
	float RecoveryPitchRemaining = 0.0f;
	bool bHasRecoilBaseline = false;
	bool bIsRecoveringRecoil = false;
};
