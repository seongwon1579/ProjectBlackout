// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOProjectile.h"
#include "BOShrewdArrowExplosive.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API ABOShrewdArrowExplosive : public ABOProjectile
{
	GENERATED_BODY()

public:
	ABOShrewdArrowExplosive();

protected:
	
	virtual void Launch(const FVector& Velocity);
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit) override;
	
	void ExecuteExplosionCue(const FHitResult& Hit);
	void ApplyImpactDamage(const FHitResult& Hit);
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Arrow", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	FGameplayTag ExplosionCueTag;
};
