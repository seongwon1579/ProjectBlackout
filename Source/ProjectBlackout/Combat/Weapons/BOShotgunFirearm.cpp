#include "Combat/Weapons/BOShotgunFirearm.h"

#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "Interfaces/BlackoutDamageable.h"

namespace
{
	FGameplayEffectSpecHandle DuplicateGameplayEffectSpec(const FGameplayEffectSpecHandle& SourceSpecHandle)
	{
		FGameplayEffectSpecHandle DuplicatedSpecHandle;
		if (SourceSpecHandle.IsValid())
		{
			DuplicatedSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*SourceSpecHandle.Data.Get());
		}
		return DuplicatedSpecHandle;
	}
}

ABOShotgunFirearm::ABOShotgunFirearm()
{
	bUseHitscan = true;
}

bool ABOShotgunFirearm::InitializeStatsFromDataTable()
{
	if (const FBlackoutShotgunFirearmStat* FoundStats = ShotgunStatsRow.GetRow<FBlackoutShotgunFirearmStat>(TEXT("BOShotgunFirearm::InitializeStatsFromDataTable")))
	{
		CachedShotgunStats = *FoundStats;
		CachedFirearmStats = CachedShotgunStats;
		ApplyCommonStats(CachedShotgunStats);
		return true;
	}

	return Super::InitializeStatsFromDataTable();
}

TArray<FBlackoutShotgunPelletHit> ABOShotgunFirearm::FireShotgun(const FVector& BaseDirection, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	TArray<FBlackoutShotgunPelletHit> PelletHits;
	UWorld* World = GetWorld();
	if (!World)
	{
		return PelletHits;
	}

	const TArray<FVector> PelletDirections = BuildPelletDirections(BaseDirection);
	PelletHits.Reserve(PelletDirections.Num());

	const FVector TraceStart = GetMuzzleTransform().GetLocation();
	const float TraceDistance = GetPelletTraceDistance();
	const float DamagePerPellet = GetDamagePerPellet();
	const bool bUseTargetCap = ShouldApplySingleTargetPelletCap();
	const int32 MaxPelletsPerTarget = GetMaxPelletsPerTarget();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOShotgunFirearm_FireShotgun), false, GetOwner());
	QueryParams.AddIgnoredActor(this);

	TMap<AActor*, int32> AppliedPelletCounts;

	for (int32 PelletIndex = 0; PelletIndex < PelletDirections.Num(); ++PelletIndex)
	{
		const FVector TraceEnd = TraceStart + PelletDirections[PelletIndex].GetSafeNormal() * TraceDistance;

		FBlackoutShotgunPelletHit PelletHit;
		PelletHit.PelletIndex = PelletIndex;
		World->LineTraceSingleByChannel(PelletHit.HitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams);

		if (bDrawDebugHitscanRay)
		{
			const bool bHit = PelletHit.HitResult.bBlockingHit;
			const FVector DebugEnd = bHit ? PelletHit.HitResult.ImpactPoint : TraceEnd;
			DrawDebugLine(World, TraceStart, DebugEnd, bHit ? FColor::Red : FColor::Green, false, DebugHitscanRayDuration, 0, DebugHitscanRayThickness);
		}

		if (!HasAuthority() || !PelletHit.HitResult.bBlockingHit || !DamageSpecHandle.IsValid())
		{
			PelletHits.Add(PelletHit);
			continue;
		}

		UPrimitiveComponent* HitComponent = PelletHit.HitResult.GetComponent();
		AActor* HitActor = PelletHit.HitResult.GetActor();
		UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent);
		AActor* DamageTargetActor = HitboxComponent ? HitboxComponent->GetOwner() : HitActor;
		if (!DamageTargetActor)
		{
			PelletHits.Add(PelletHit);
			continue;
		}

		if (bUseTargetCap)
		{
			int32& AppliedCount = AppliedPelletCounts.FindOrAdd(DamageTargetActor);
			if (AppliedCount >= MaxPelletsPerTarget)
			{
				PelletHits.Add(PelletHit);
				continue;
			}
			++AppliedCount;
		}

		FGameplayEffectSpecHandle PelletSpecHandle = DuplicateGameplayEffectSpec(DamageSpecHandle);
		if (!PelletSpecHandle.IsValid())
		{
			PelletHits.Add(PelletHit);
			continue;
		}

		if (HitboxComponent)
		{
			PelletHit.HitPartTag = HitboxComponent->GetPartTag();
			PelletHit.DamageApplied = DamagePerPellet * HitboxComponent->GetDamageMultiplier();
			HitboxComponent->ReceiveDamageSpec(PelletSpecHandle);
			PelletHit.bAppliedDamage = true;
		}
		else if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
		{
			PelletHit.HitPartTag = Damageable->GetHitPartTag(PelletHit.HitResult.BoneName);
			PelletHit.DamageApplied = DamagePerPellet;
			Damageable->ReceiveDamageFromHitbox(PelletSpecHandle, PelletHit.HitResult.BoneName);
			PelletHit.bAppliedDamage = true;
		}

		PelletHits.Add(PelletHit);
	}

	return PelletHits;
}

TArray<FVector> ABOShotgunFirearm::BuildPelletDirections(const FVector& BaseDirection) const
{
	TArray<FVector> PelletDirections;
	const int32 PelletCount = GetPelletCount();
	PelletDirections.Reserve(PelletCount);

	const FVector SafeBaseDirection = BaseDirection.GetSafeNormal();
	if (SafeBaseDirection.IsNearlyZero())
	{
		return PelletDirections;
	}

	const float SpreadRadians = FMath::DegreesToRadians(GetPelletSpreadDegrees());
	for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
	{
		PelletDirections.Add(FMath::VRandCone(SafeBaseDirection, SpreadRadians));
	}

	return PelletDirections;
}

int32 ABOShotgunFirearm::GetPelletCount() const
{
	return FMath::Max(CachedShotgunStats.PelletCount, 1);
}

float ABOShotgunFirearm::GetPelletSpreadDegrees() const
{
	return FMath::Max(CachedShotgunStats.PelletSpreadDegrees, 0.0f);
}

float ABOShotgunFirearm::GetPelletTraceDistance() const
{
	return FMath::Max(CachedShotgunStats.PelletTraceDistance, 0.0f);
}

float ABOShotgunFirearm::GetDamagePerPellet() const
{
	if (CachedShotgunStats.DamagePerPellet > 0.0f)
	{
		return CachedShotgunStats.DamagePerPellet;
	}

	return GetBaseDamage() / static_cast<float>(GetPelletCount());
}

bool ABOShotgunFirearm::ShouldApplySingleTargetPelletCap() const
{
	return CachedShotgunStats.bSingleTargetPelletCap;
}

int32 ABOShotgunFirearm::GetMaxPelletsPerTarget() const
{
	return FMath::Max(CachedShotgunStats.MaxPelletsPerTarget, 1);
}
