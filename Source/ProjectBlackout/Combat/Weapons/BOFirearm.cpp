#include "Combat/Weapons/BOFirearm.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Weapons/BOProjectile.h"

ABOFirearm::ABOFirearm()
{
	MuzzleFlash = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleFlash"));
	MuzzleFlash->SetupAttachment(WeaponMesh);
	MuzzleFlash->SetAutoActivate(false);
	
	MuzzleSocket = TEXT("MuzzleSocket");
}

FHitResult ABOFirearm::Fire(const FVector& Direction)
{
	FHitResult HitResult;
	if (bUseHitscan)
	{
		// TODO: Implement LineTraceByChannel
	}
	else
	{
		SpawnProjectile(Direction);
	}
	return HitResult;
}

ABOProjectile* ABOFirearm::SpawnProjectile(const FVector& Direction)
{
	// TODO: Spawn from UBlackoutPoolSubsystem instead of normal spawn
	return nullptr;
}

FTransform ABOFirearm::GetMuzzleTransform() const
{
	if (WeaponMesh)
	{
		return WeaponMesh->GetSocketTransform(MuzzleSocket);
	}
	return GetActorTransform();
}
