#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BlackoutTypes.h"
#include "GameplayTagContainer.h"
#include "BlackoutCombatComponent.generated.h"

class ABOWeaponBase;
class ABOFirearm;
class ABOMeleeWeapon;
class UBOCharacterData;

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
	void PerformMeleeHit();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

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

protected:
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_IsAiming();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOWeaponBase> EquippedWeapon;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> PrimaryWeapon;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> SecondaryWeapon;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOMeleeWeapon> MeleeWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bIsAiming = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float AimParallaxOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float AimTraceDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName EquippedWeaponSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName PrimaryHolsterSocketName = TEXT("PrimaryWeaponSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName SecondaryHolsterSocketName = TEXT("SecondaryWeaponSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName MeleeHolsterSocketName = TEXT("MeleeWeaponSocket");

private:
	ABOWeaponBase* SpawnWeaponActor(TSubclassOf<ABOWeaponBase> WeaponClass);
	void RefreshWeaponAttachments() const;
	void ApplyInitialAmmoLoadout() const;
	bool CanStartAim() const;
	float GetEquippedClipAmmo() const;
	void ApplyAimingState(bool bNewAiming);
	EBlackoutAbilityInputID ResolvePrimaryActionInputID() const;
	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID) const;
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID) const;

	UPROPERTY(Transient)
	EBlackoutAbilityInputID ActivePrimaryActionInputID = EBlackoutAbilityInputID::None;
};
