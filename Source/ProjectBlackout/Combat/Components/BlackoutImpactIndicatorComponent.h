#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutImpactIndicatorComponent.generated.h"

class ABOFirearm;
class UBlackoutCombatComponent;
struct FPredictProjectilePathParams;
struct FPredictProjectilePathResult;

// 착탄 예측 캐시를 무효화할지 판단하는 최소 입력 상태입니다.
struct FBlackoutImpactIndicatorUpdateKey
{
	bool bIsAiming = false;
	FRotator CameraRotation = FRotator::ZeroRotator;
	FVector MuzzleLocation = FVector::ZeroVector;
	TWeakObjectPtr<ABOFirearm> EquippedFirearm;
};

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
	bool BuildImpactIndicatorUpdateKey(FBlackoutImpactIndicatorUpdateKey& OutUpdateKey) const;
	bool HasImpactIndicatorUpdateInputChanged(const FBlackoutImpactIndicatorUpdateKey& UpdateKey) const;
	bool RefreshCachedImpactIndicatorData(const FBlackoutImpactIndicatorUpdateKey& UpdateKey, float SpreadNormalized) const;
	bool GetTrueImpactPointInternal(FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutProjectileTravelDistance, TArray<FBlackoutTrajectoryPointData>* OutTrajectoryPoints) const;
	bool GetHitscanImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd) const;
	bool GetProjectileImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutTravelDistance, TArray<FBlackoutTrajectoryPointData>* OutTrajectoryPoints) const;
	bool BuildProjectilePathParams(const ABOFirearm* Firearm, const FVector& LaunchDirection, FPredictProjectilePathParams& OutParams) const;
	void BuildTrajectoryPoints(const FPredictProjectilePathResult& PathResult, TArray<FBlackoutTrajectoryPointData>& OutTrajectoryPoints, float& OutTravelDistance) const;
	bool PerformWeaponTrace(const FVector& TraceStart, const FVector& TraceEnd, const AActor* IgnoredActor, FHitResult& OutHitResult) const;
	bool IsProjectileImpactOccludedFromCamera(const ABOFirearm* Firearm, const FVector& ImpactPoint) const;
	AActor* ResolveTargetActor(const FHitResult& HitResult) const;
	FVector ResolveFireDirection(const ABOFirearm* Firearm) const;

	UPROPERTY(Transient)
	TWeakObjectPtr<UBlackoutCombatComponent> CombatComponent;

	// GetImpactIndicatorData()가 매 틱 호출되어도 입력이 같으면 이전 계산 결과를 재사용합니다.
	mutable bool bHasCachedIndicatorData = false;
	mutable FBlackoutImpactIndicatorUpdateKey LastUpdateKey;
	mutable FBlackoutImpactIndicatorData CachedIndicatorData;
};
