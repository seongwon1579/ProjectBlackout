// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_Base.h"
#include "BlackoutGA_Shrewd_FireArrowBase.generated.h"

/**
 * 
 */
class ABOProjectile;
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_FireArrowBase : public UBlackoutGA_Shrewd_Base
{
	GENERATED_BODY()
	
protected:
	virtual void SetupEventListeners() override;

	UFUNCTION()
	void OnFireShotEvent(FGameplayEventData Payload);
	
	
	virtual void LaunchProjectile(
		ABOProjectile* Arrow,
		const FVector& SpawnLocation,
		const FVector& TargetLocation){}
	
	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile")
	FName SpawnSocketName = TEXT("BlackoutFirePoint");

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Damage")
	float DamageMagnitude = 0.f;
	
};
