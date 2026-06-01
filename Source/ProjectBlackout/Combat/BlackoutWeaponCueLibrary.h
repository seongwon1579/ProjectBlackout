#pragma once

#include "CoreMinimal.h"
#include "Data/BlackoutWeaponStat.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlackoutWeaponCueLibrary.generated.h"

class UAbilitySystemComponent;

/**
 * 무기별 GCN 태그 해석과 서버 실행을 담당하는 공통 유틸리티.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutWeaponCueLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	static FGameplayTag ResolveSurfaceTag(const FHitResult& HitResult);

	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	static FGameplayTag ResolveImpactCueTag(const FBlackoutWeaponCueSet& CueSet, FGameplayTag SurfaceTag);

	static FGameplayCueParameters BuildFireCueParameters(AActor* SourceActor, const FVector& MuzzleLocation, const FVector& FireDirection);
	static FGameplayCueParameters BuildTrailCueParameters(AActor* SourceActor, const FVector& MuzzleLocation, const FHitResult& HitResult, const FVector& TraceEnd);
	static FGameplayCueParameters BuildImpactCueParameters(AActor* SourceActor, const FHitResult& HitResult);

	static void ExecuteWeaponCue(UAbilitySystemComponent* SourceASC, FGameplayTag CueTag, const FGameplayCueParameters& CueParameters);
	static void ExecuteFireCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FVector& MuzzleLocation, const FVector& FireDirection);
	static void ExecuteTrailCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FVector& MuzzleLocation, const FHitResult& HitResult, const FVector& TraceEnd);
	static void ExecuteImpactCue(UAbilitySystemComponent* SourceASC, const FBlackoutWeaponCueSet& CueSet, AActor* SourceActor, const FHitResult& HitResult);
};
