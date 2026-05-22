// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/BORavagerData.h"

#include "BOEnemyProjectile.generated.h"

class UNiagaraComponent;
class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOEnemyProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABOEnemyProjectile();
	
	virtual void InitializeProjectile(const FProjectileSpawnData& InSpawnParams);
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	virtual void ApplyDamageToTarget(AActor* Target, FName HitBoneName);
	virtual bool ShouldIgnoreHit(AActor* OtherActor) const;
	virtual void SetCollisionEvent();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> CollisionComp;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> Effect;
	
	UPROPERTY(Transient)
	FProjectileSpawnData SpawnParams;
};
