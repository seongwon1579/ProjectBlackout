// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/BORavagerData.h"

#include "BOShockwaveProjectile.generated.h"


UCLASS()
class PROJECTBLACKOUT_API ABOShockwaveProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABOShockwaveProjectile();
	
	void InitializeProjectile(const FProjectileSpawnParams& InSpawnParams);
	
protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			   const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> CollisionComp;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> Effect;
	
	UPROPERTY(Transient)
	FProjectileSpawnParams SpawnParams;
};
