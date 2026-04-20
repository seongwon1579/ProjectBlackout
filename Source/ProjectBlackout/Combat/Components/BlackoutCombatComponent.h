#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlackoutCombatComponent.generated.h"

class ABOWeaponBase;
class ABOFirearm;
class ABOMeleeWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBlackoutCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	void StartAim();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void StopAim();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void TryReload();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void PerformMeleeHit();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FVector GetAimImpactPoint() const;

	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(ABOWeaponBase* NewWeapon);

protected:
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOWeaponBase> EquippedWeapon;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> PrimaryWeapon;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOFirearm> SecondaryWeapon;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<ABOMeleeWeapon> MeleeWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bIsAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float AimParallaxOffset = 100.0f;
};
