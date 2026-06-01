#include "Combat/BlackoutWeaponCueLibrary.h"

#include "AbilitySystemComponent.h"
#include "Combat/BlackoutImpactSurfaceSettings.h"
#include "Core/BlackoutLog.h"

FGameplayTag UBlackoutWeaponCueLibrary::ResolveSurfaceTag(const FHitResult& HitResult)
{
	const UBlackoutImpactSurfaceSettings* SurfaceSettings = UBlackoutImpactSurfaceSettings::Get();
	return SurfaceSettings ? SurfaceSettings->ResolveSurfaceTag(HitResult.PhysMaterial.Get()) : FGameplayTag();
}

FGameplayTag UBlackoutWeaponCueLibrary::ResolveImpactCueTag(const FBlackoutWeaponCueSet& CueSet, FGameplayTag SurfaceTag)
{
	return CueSet.ResolveImpactCue(SurfaceTag);
}

FGameplayCueParameters UBlackoutWeaponCueLibrary::BuildFireCueParameters(AActor* SourceActor, const FVector& MuzzleLocation, const FVector& FireDirection)
{
	FGameplayCueParameters CueParameters;
	CueParameters.Location = MuzzleLocation;
	CueParameters.Normal = FireDirection.GetSafeNormal();
	CueParameters.Instigator = SourceActor ? SourceActor->GetInstigator() : nullptr;
	CueParameters.EffectCauser = SourceActor;
	CueParameters.SourceObject = SourceActor;
	return CueParameters;
}

FGameplayCueParameters UBlackoutWeaponCueLibrary::BuildTrailCueParameters(AActor* SourceActor, const FVector& MuzzleLocation, const FHitResult& HitResult, const FVector& TraceEnd)
{
	const FVector TrailEnd = HitResult.bBlockingHit ? FVector(HitResult.ImpactPoint) : TraceEnd;
	const FVector TrailVector = TrailEnd - MuzzleLocation;

	FGameplayCueParameters CueParameters;
	CueParameters.Location = MuzzleLocation;
	CueParameters.Normal = TrailVector.GetSafeNormal();
	CueParameters.RawMagnitude = TrailVector.Length();
	CueParameters.PhysicalMaterial = HitResult.PhysMaterial.Get();
	CueParameters.Instigator = SourceActor ? SourceActor->GetInstigator() : nullptr;
	CueParameters.EffectCauser = SourceActor;
	CueParameters.SourceObject = SourceActor;
	return CueParameters;
}

FGameplayCueParameters UBlackoutWeaponCueLibrary::BuildImpactCueParameters(AActor* SourceActor, const FHitResult& HitResult)
{
	FGameplayCueParameters CueParameters;
	CueParameters.Location = HitResult.bBlockingHit ? FVector(HitResult.ImpactPoint) : FVector(HitResult.TraceEnd);
	CueParameters.Normal = HitResult.ImpactNormal;
	CueParameters.PhysicalMaterial = HitResult.PhysMaterial.Get();
	CueParameters.Instigator = SourceActor ? SourceActor->GetInstigator() : nullptr;
	CueParameters.EffectCauser = SourceActor;
	CueParameters.SourceObject = SourceActor;
	return CueParameters;
}

void UBlackoutWeaponCueLibrary::ExecuteWeaponCue(UAbilitySystemComponent* SourceASC, FGameplayTag CueTag, const FGameplayCueParameters& CueParameters)
{
	if (!CueTag.IsValid())
	{
		BO_LOG_CORE(Warning, "ExecuteWeaponCue skipped: CueTag가 유효하지 않음");
		return;
	}

	if (!SourceASC)
	{
		BO_LOG_CORE(Error, "ExecuteWeaponCue failed: SourceASC가 유효하지 않음 (Cue=%s)", *CueTag.ToString());
		return;
	}

	SourceASC->ExecuteGameplayCue(CueTag, CueParameters);
}

void UBlackoutWeaponCueLibrary::ExecuteFireCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FVector& MuzzleLocation, const FVector& FireDirection)
{
	ExecuteWeaponCue(SourceASC, CueSet.FireCueTag, BuildFireCueParameters(SourceActor, MuzzleLocation, FireDirection));
}

void UBlackoutWeaponCueLibrary::ExecuteTrailCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FVector& MuzzleLocation, const FHitResult& HitResult, const FVector& TraceEnd)
{
	ExecuteWeaponCue(SourceASC, CueSet.TrailCueTag, BuildTrailCueParameters(SourceActor, MuzzleLocation, HitResult, TraceEnd));
}

void UBlackoutWeaponCueLibrary::ExecuteImpactCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FHitResult& HitResult)
{
	if (!HitResult.bBlockingHit)
	{
		return;
	}

	const FGameplayTag SurfaceTag = ResolveSurfaceTag(HitResult);
	const FGameplayTag ImpactCueTag = ResolveImpactCueTag(CueSet, SurfaceTag);
	ExecuteWeaponCue(SourceASC, ImpactCueTag, BuildImpactCueParameters(SourceActor, HitResult));
}
