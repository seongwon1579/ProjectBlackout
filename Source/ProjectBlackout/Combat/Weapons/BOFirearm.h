#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOWeaponBase.h"
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
	FHitResult Fire(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOProjectile* SpawnProjectile(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	virtual bool InitializeStatsFromDataTable() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetFireRate() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool IsAutomatic() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMaxReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetSplashRadius() const;

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
};
