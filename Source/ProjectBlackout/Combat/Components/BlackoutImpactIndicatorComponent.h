#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutImpactIndicatorComponent.generated.h"

class ABOFirearm;
class UBlackoutCombatComponent;
struct FPredictProjectilePathParams;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutImpactIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBlackoutImpactIndicatorComponent();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void Initialize(UBlackoutCombatComponent* InCombatComponent);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool GetImpactIndicatorData(FBlackoutImpactIndicatorData& OutIndicatorData) const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool GetAimTargetHitResult(FHitResult& OutHitResult, FVector& OutTraceEnd) const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FVector GetAimTargetPoint() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool GetTrueImpactPoint(FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 100.0f))
	float MaxTraceDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 0.1f))
	float ProjectilePredictionTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 1.0f))
	float ProjectilePredictionSimFrequency = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 0.0f))
	float ProjectilePredictionRadiusFallback = 5.0f;

private:
	UBlackoutCombatComponent* ResolveCombatComponent() const;
	bool GetTrueImpactPointInternal(FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutProjectileTravelDistance) const;
	bool GetHitscanImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd) const;
	bool GetProjectileImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutTravelDistance) const;
	bool BuildProjectilePathParams(const ABOFirearm* Firearm, const FVector& LaunchDirection, FPredictProjectilePathParams& OutParams) const;
	bool PerformWeaponTrace(const FVector& TraceStart, const FVector& TraceEnd, const AActor* IgnoredActor, FHitResult& OutHitResult) const;
	bool IsProjectileImpactOccludedFromCamera(const ABOFirearm* Firearm, const FVector& ImpactPoint) const;
	AActor* ResolveTargetActor(const FHitResult& HitResult) const;
	FVector ResolveFireDirection(const ABOFirearm* Firearm) const;

	UPROPERTY(Transient)
	TWeakObjectPtr<UBlackoutCombatComponent> CombatComponent;
};
