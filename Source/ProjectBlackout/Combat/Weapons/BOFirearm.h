#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "GameplayEffectTypes.h"
#include "BOFirearm.generated.h"

class UNiagaraComponent;
class ABOProjectile;

UCLASS()
class PROJECTBLACKOUT_API ABOFirearm : public ABOWeaponBase
{
	GENERATED_BODY()
	
public:
	ABOFirearm();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FHitResult Fire(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOProjectile* SpawnProjectile(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	virtual bool InitializeStatsFromDataTable() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetFireRate() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool IsAutomatic() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool UsesHitscan() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMaxReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetSplashRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> GetProjectileClass() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileLaunchSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileGravityScale() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileCollisionRadius() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UNiagaraComponent> MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (RowType = "/Script/ProjectBlackout.BlackoutFirearmStat"))
	FDataTableRowHandle FirearmStatsRow;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Weapon")
	FBlackoutFirearmStat CachedFirearmStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName MuzzleSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bUseHitscan = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bDrawDebugHitscanRay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugHitscanRay", ClampMin = 0.f))
	float DebugHitscanRayDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugHitscanRay", ClampMin = 0.f))
	float DebugHitscanRayThickness = 1.5f;
};
