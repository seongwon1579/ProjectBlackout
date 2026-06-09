// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapons/BOEnemySpawnerProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"


ABOEnemySpawnerProjectile::ABOEnemySpawnerProjectile()
{
	ProjectileMovement->ProjectileGravityScale = 1.f;
}

void ABOEnemySpawnerProjectile::SetSpawnerData(const FMinionSpawnData& InData)
{
	MinionData = InData;
}

void ABOEnemySpawnerProjectile::SetCollisionEvent()
{
	if (CollisionComp)
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &ABOEnemySpawnerProjectile::OnHit);
	}
}

void ABOEnemySpawnerProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                      FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShouldIgnoreHit(OtherActor)) return;
	if (!HasAuthority()) return;
	if (bHasLanded) return;

	bHasLanded = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimer(
		HatchTimer,
		this,
		&ABOEnemySpawnerProjectile::Hatch,
		MinionData.HatchDelay,
		false);
}

void ABOEnemySpawnerProjectile::Hatch()
{
	UWorld* World = GetWorld();
	if (!World || !MinionData.MinionClass)
	{
		Destroy();
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = GetOwner();
	Params.Instigator = GetInstigator();

	FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 50);
	FRotator SpawnRotation(0.f, GetActorRotation().Yaw, 0.f);

	ACharacter* Minion = World->SpawnActor<ACharacter>
	(
		MinionData.MinionClass,
			SpawnLocation,
			SpawnRotation,
			Params);
	
	if (Minion)
	{
		Minion->SpawnDefaultController();
	}
	
	Destroy();
}
