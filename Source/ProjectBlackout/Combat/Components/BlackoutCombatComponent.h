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
class UBOCharacterData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutEquippedWeaponChangedSignature, ABOWeaponBase*, EquippedWeapon, FGameplayTag, WeaponSlotTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBlackoutCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	ABOWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Combat")
	FBlackoutEquippedWeaponChangedSignature OnEquippedWeaponChanged;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOFirearm* GetEquippedFirearm() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOMeleeWeapon* GetMeleeWeapon() const { return MeleeWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayTag GetEquippedWeaponSlotTag() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FVector GetAimImpactPoint() const;

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

protected:
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_LoadoutWeapon();

	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION()
	void OnRep_MeleeWeaponAttachmentOverride();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOWeaponBase> EquippedWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> PrimaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> SecondaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LoadoutWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOMeleeWeapon> MeleeWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bIsAiming = false;

	UPROPERTY(ReplicatedUsing = OnRep_MeleeWeaponAttachmentOverride, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bMeleeWeaponAttachmentOverride = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float AimParallaxOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float AimTraceDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName EquippedWeaponSocketName = TEXT("WeaponSocket");

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

	UPROPERTY(Transient)
	EBlackoutAbilityInputID ActivePrimaryActionInputID = EBlackoutAbilityInputID::None;

	UPROPERTY(Transient)
	FGameplayTag PendingWeaponSwapSlotTag;

	UPROPERTY(Transient)
	bool bIsWeaponSwapInProgress = false;

	FTimerHandle AutomaticFireTimerHandle;
};
