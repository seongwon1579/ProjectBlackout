#include "Combat/Weapons/BOFirearm.h"

#include "Engine/World.h"
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

bool ABOFirearm::InitializeStatsFromDataTable()
{
	Super::InitializeStatsFromDataTable();

	if (const FBlackoutFirearmStat* FoundStats = FirearmStatsRow.GetRow<FBlackoutFirearmStat>(TEXT("BOFirearm::InitializeStatsFromDataTable")))
	{
		CachedFirearmStats = *FoundStats;
		ApplyCommonStats(CachedFirearmStats);
		return true;
	}

	return false;
}

FHitResult ABOFirearm::Fire(const FVector& Direction)
{
	FHitResult HitResult;
	if (bUseHitscan)
	{
		if (UWorld* World = GetWorld())
		{
			const FVector TraceStart = GetMuzzleTransform().GetLocation();
			const FVector TraceEnd = TraceStart + Direction.GetSafeNormal() * 10000.0f;

			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOFirearm_Fire), false, GetOwner());
			QueryParams.AddIgnoredActor(this);

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		}
	}
	else
	{
		SpawnProjectile(Direction);
	}
	return HitResult;
}

ABOProjectile* ABOFirearm::SpawnProjectile(const FVector& Direction)
{
	// TODO: 일반 스폰 대신 UBlackoutPoolSubsystem에서 스폰하도록 구현
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

float ABOFirearm::GetFireRate() const
{
	return CachedFirearmStats.FireRate;
}

bool ABOFirearm::IsAutomatic() const
{
	return CachedFirearmStats.bIsAutomatic;
}

int32 ABOFirearm::GetMagazineSize() const
{
	return CachedFirearmStats.MagazineSize;
}

int32 ABOFirearm::GetMaxReserveAmmo() const
{
	return CachedFirearmStats.MaxReserveAmmo;
}

float ABOFirearm::GetSplashRadius() const
{
	return CachedFirearmStats.SplashRadius;
}
