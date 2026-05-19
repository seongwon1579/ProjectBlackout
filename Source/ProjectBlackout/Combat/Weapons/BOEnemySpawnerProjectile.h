// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOEnemyProjectile.h"
#include "BOEnemySpawnerProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API ABOEnemySpawnerProjectile : public ABOEnemyProjectile
{
	GENERATED_BODY()

public:

	void SetSpawnerData(const FMinionSpawnData& InData);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit) override;
	
	void Hatch();
	
	UPROPERTY(Transient)
	FMinionSpawnData MinionData;
	
	UPROPERTY(Transient)
	FTimerHandle HatchTimer;
	
	UPROPERTY(Transient)
	bool bHasLanded = false;
	
	
};
