#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "GameplayEffectTypes.h"
#include "BOMeleeWeapon.generated.h"

class UBoxComponent;
class UGameplayEffect;
class UPrimitiveComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOMeleeWeapon : public ABOWeaponBase
{
	GENERATED_BODY()
	
public:
	ABOMeleeWeapon();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TArray<FHitResult> PerformSweep(const FVector& Forward);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void SetHitBoxActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void BeginHitWindow(TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void EndHitWindow();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TArray<FHitResult> PerformSweepHit(const FVector& Forward, TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	bool IsHitWindowActive() const { return bHitBoxActive; }

	virtual bool InitializeStatsFromDataTable() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (RowType = "/Script/ProjectBlackout.BlackoutMeleeWeaponStat"))
	FDataTableRowHandle MeleeStatsRow;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Weapon")
	FBlackoutMeleeWeaponStat CachedMeleeStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float SwingRadius = 50.0f;

	UFUNCTION()
	void HandleHitBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	FGameplayEffectSpecHandle BuildDamageSpec(TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel) const;
	bool ApplyDamageToTarget(
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FName BoneName,
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		float EffectLevel,
		TSet<AActor*>& DamagedActors) const;

	TSet<AActor*> HitActorsThisWindow;
	TSubclassOf<UGameplayEffect> ActiveDamageEffectClass;
	float ActiveDamageEffectLevel = 1.0f;
	bool bHitBoxActive = false;
};
