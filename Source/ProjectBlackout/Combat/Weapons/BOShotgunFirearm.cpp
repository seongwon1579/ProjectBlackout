#include "Combat/Weapons/BOShotgunFirearm.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Weapons/BOWeaponDebugUtils.h"
#include "Core/BlackoutCollisionChannels.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GAS/Effects/ExecCalc_CombatReward.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"

ABOShotgunFirearm::ABOShotgunFirearm()
{
	bUseHitscan = true;
}

bool ABOShotgunFirearm::InitializeStatsFromDataTable()
{
	if (const FBlackoutShotgunFirearmStat* FoundStats = ShotgunStatsRow.GetRow<FBlackoutShotgunFirearmStat>(TEXT("BOShotgunFirearm::InitializeStatsFromDataTable")))
	{
		CachedShotgunStats = *FoundStats;
		ApplyFirearmStats(CachedShotgunStats);
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
	QueryParams.bReturnPhysicalMaterial = true;

	TMap<AActor*, int32> AppliedPelletCounts;
	TArray<TPair<TWeakObjectPtr<AActor>, FGameplayEffectSpecHandle>> KilledTargets;
	TSet<AActor*> KilledTargetSet;

	for (int32 PelletIndex = 0; PelletIndex < PelletDirections.Num(); ++PelletIndex)
	{
		const FVector TraceEnd = TraceStart + PelletDirections[PelletIndex].GetSafeNormal() * TraceDistance;

		FBlackoutShotgunPelletHit PelletHit;
		PelletHit.PelletIndex = PelletIndex;
		World->LineTraceSingleByChannel(PelletHit.HitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams);

		UPrimitiveComponent* HitComponent = PelletHit.HitResult.GetComponent();
		AActor* HitActor = PelletHit.HitResult.GetActor();
		UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent);
		AActor* DamageTargetActor = BlackoutWeaponDebug::ResolveDamageTargetActor(PelletHit.HitResult);

		if (bDrawDebugHitscanRay)
		{
			const bool bHit = PelletHit.HitResult.bBlockingHit;
			const FVector DebugEnd = bHit ? PelletHit.HitResult.ImpactPoint : TraceEnd;
			const FColor DebugColor = BlackoutWeaponDebug::GetHitscanDebugColor(bHit, DamageTargetActor);
			DrawDebugLine(World, TraceStart, DebugEnd, DebugColor, false, DebugHitscanRayDuration, 0, DebugHitscanRayThickness);
		}

		if (!HasAuthority() || !PelletHit.HitResult.bBlockingHit || !DamageSpecHandle.IsValid())
		{
			PelletHits.Add(PelletHit);
			continue;
		}

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

		float HealthBeforeDamage = 0.0f;
		const bool bHadHealthBefore = BlackoutWeaponDebug::TryGetHealth(DamageTargetActor, HealthBeforeDamage);

		FGameplayEffectSpecHandle PelletSpecHandle = BlackoutWeaponDebug::DuplicateGameplayEffectSpec(DamageSpecHandle);
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

		if (PelletHit.bAppliedDamage && bHadHealthBefore && HealthBeforeDamage > 0.0f)
		{
			float HealthAfterDamage = 0.0f;
			if (BlackoutWeaponDebug::TryGetHealth(DamageTargetActor, HealthAfterDamage)
				&& HealthAfterDamage <= 0.0f
				&& !KilledTargetSet.Contains(DamageTargetActor))
			{
				KilledTargetSet.Add(DamageTargetActor);
				KilledTargets.Add(TPair<TWeakObjectPtr<AActor>, FGameplayEffectSpecHandle>(DamageTargetActor, PelletSpecHandle));
			}
		}

		PelletHits.Add(PelletHit);
	}

	if (KilledTargets.Num() >= 3)
	{
		for (const TPair<TWeakObjectPtr<AActor>, FGameplayEffectSpecHandle>& KilledTarget : KilledTargets)
		{
			AActor* TargetActor = KilledTarget.Key.Get();
			IAbilitySystemInterface* TargetAbilityInterface = Cast<IAbilitySystemInterface>(TargetActor);
			UAbilitySystemComponent* TargetASC = TargetAbilityInterface ? TargetAbilityInterface->GetAbilitySystemComponent() : nullptr;
			FGameplayEffectSpecHandle RewardSpecHandle = BlackoutWeaponDebug::DuplicateGameplayEffectSpec(KilledTarget.Value);
			if (TargetASC && RewardSpecHandle.IsValid())
			{
				// 산탄 한 번으로 3마리 이상 처치가 확정된 뒤 데몰리션 보상 조건 태그를 후처리합니다.
				RewardSpecHandle.Data->AddDynamicAssetTag(BlackoutGameplayTags::Kill_MultiTarget_Count3);
				UExecCalc_CombatReward::ApplyConfiguredRewardEffect(RewardSpecHandle, TargetASC);
			}
		}
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
