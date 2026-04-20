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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UNiagaraComponent> MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName MuzzleSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bUseHitscan = true;
};
