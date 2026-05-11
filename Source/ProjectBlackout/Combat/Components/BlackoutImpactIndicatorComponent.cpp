#include "Combat/Components/BlackoutImpactIndicatorComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutLog.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Components/PrimitiveComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

namespace
{
	// 카메라 떨림과 소켓의 미세한 흔들림 때문에 매 프레임 캐시가 무효화되지 않도록 허용 오차를 둡니다.
	constexpr float ImpactIndicatorCameraRotationToleranceDegrees = 0.05f;
	constexpr float ImpactIndicatorMuzzleLocationTolerance = 0.5f;

	void AppendIgnoredProjectiles(UWorld* World, TArray<TObjectPtr<AActor>>& OutIgnoredActors)
	{
		if (!World)
		{
			return;
		}

		for (TActorIterator<ABOProjectile> It(World); It; ++It)
		{
			ABOProjectile* Projectile = *It;
			if (IsValid(Projectile) && !Projectile->IsActorBeingDestroyed())
			{
				OutIgnoredActors.AddUnique(Projectile);
			}
		}
	}

	void AddIgnoredProjectiles(UWorld* World, FCollisionQueryParams& QueryParams)
	{
		if (!World)
		{
			return;
		}

		for (TActorIterator<ABOProjectile> It(World); It; ++It)
		{
			ABOProjectile* Projectile = *It;
			if (IsValid(Projectile) && !Projectile->IsActorBeingDestroyed())
			{
				QueryParams.AddIgnoredActor(Projectile);
			}
		}
	}
}

UBlackoutImpactIndicatorComponent::UBlackoutImpactIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBlackoutImpactIndicatorComponent::Initialize(UBlackoutCombatComponent* InCombatComponent)
{
	CombatComponent = InCombatComponent;
	bHasCachedIndicatorData = false;
	CachedIndicatorData = FBlackoutImpactIndicatorData();
}

bool UBlackoutImpactIndicatorComponent::GetImpactIndicatorData(FBlackoutImpactIndicatorData& OutIndicatorData) const
{
	OutIndicatorData = FBlackoutImpactIndicatorData();

	const UBlackoutCombatComponent* ResolvedCombatComponent = ResolveCombatComponent();
	const float SpreadNormalized = ResolvedCombatComponent ? ResolvedCombatComponent->GetNormalizedSpread() : 0.0f;

	FBlackoutImpactIndicatorUpdateKey UpdateKey;
	if (!BuildImpactIndicatorUpdateKey(UpdateKey))
	{
		bHasCachedIndicatorData = false;
		CachedIndicatorData = FBlackoutImpactIndicatorData();
		OutIndicatorData.SpreadNormalized = SpreadNormalized;
		return false;
	}

	if (!bHasCachedIndicatorData || HasImpactIndicatorUpdateInputChanged(UpdateKey))
	{
		(bool)RefreshCachedImpactIndicatorData(UpdateKey, SpreadNormalized);
	}

	// 탄퍼짐은 매 틱 달라질 수 있으므로 착탄 예측을 다시 돌리지 않고 캐시에 값만 덮어씁니다.
	CachedIndicatorData.SpreadNormalized = SpreadNormalized;
	OutIndicatorData = CachedIndicatorData;

	return OutIndicatorData.bIsVisible;
}

bool UBlackoutImpactIndicatorComponent::RefreshCachedImpactIndicatorData(const FBlackoutImpactIndicatorUpdateKey& UpdateKey, float SpreadNormalized) const
{
	FBlackoutImpactIndicatorData RefreshedData;
	RefreshedData.SpreadNormalized = SpreadNormalized;
	LastUpdateKey = UpdateKey;
	bHasCachedIndicatorData = true;

	if (!UpdateKey.bIsAiming || UpdateKey.bIsReloading)
	{
		CachedIndicatorData = RefreshedData;
		return true;
	}

	const ABOFirearm* Firearm = UpdateKey.EquippedFirearm.Get();
	if (!Firearm)
	{
		CachedIndicatorData = RefreshedData;
		return false;
	}

	FHitResult AimHitResult;
	FVector AimTraceEnd = FVector::ZeroVector;
	if (!GetAimTargetHitResult(AimHitResult, AimTraceEnd))
	{
		CachedIndicatorData = RefreshedData;
		return false;
	}

	FHitResult TrueImpactHitResult;
	FVector ImpactPoint = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	float ProjectileTravelDistance = 0.0f;
	TArray<FBlackoutTrajectoryPointData> TrajectoryPoints;
	if (!GetTrueImpactPointInternal(TrueImpactHitResult, ImpactPoint, TraceEnd, &ProjectileTravelDistance, &TrajectoryPoints))
	{
		CachedIndicatorData = RefreshedData;
		return false;
	}

	const bool bUsesProjectilePrediction = !Firearm->UsesHitscan();
	const float DistanceFromMuzzle = FVector::Dist(Firearm->GetMuzzleTransform().GetLocation(), ImpactPoint);
	const float ProjectileImpactFuseArmDistance = Firearm->GetProjectileImpactFuseArmDistance();

	RefreshedData.bIsVisible = true;
	RefreshedData.bHasBlockingHit = TrueImpactHitResult.bBlockingHit;
	RefreshedData.bUsesProjectilePrediction = bUsesProjectilePrediction;
	RefreshedData.bProjectileImpactFuseInactive = bUsesProjectilePrediction
		&& TrueImpactHitResult.bBlockingHit
		&& ProjectileImpactFuseArmDistance > 0.0f
		&& ProjectileTravelDistance < ProjectileImpactFuseArmDistance;
	RefreshedData.bTargetMismatch = !bUsesProjectilePrediction
		&& AimHitResult.bBlockingHit
		&& ResolveTargetActor(AimHitResult) != ResolveTargetActor(TrueImpactHitResult);
	RefreshedData.bIsOccludedFromCamera = bUsesProjectilePrediction
		&& IsProjectileImpactOccludedFromCamera(Firearm, ImpactPoint);
	RefreshedData.WorldLocation = ImpactPoint;
	RefreshedData.TraceEndLocation = TraceEnd;
	RefreshedData.DistanceFromMuzzle = DistanceFromMuzzle;
	RefreshedData.TrajectoryPoints = MoveTemp(TrajectoryPoints);

	for (FBlackoutTrajectoryPointData& TrajectoryPoint : RefreshedData.TrajectoryPoints)
	{
		// HUD는 전투 조건을 다시 계산하지 않고 포인트별 상태만 보고 색상을 선택합니다.
		if (ProjectileImpactFuseArmDistance > 0.0f && TrajectoryPoint.DistanceFromMuzzle < ProjectileImpactFuseArmDistance)
		{
			TrajectoryPoint.VisualState = EBlackoutTrajectoryVisualState::FuseInactive;
		}
		else if (RefreshedData.bIsOccludedFromCamera)
		{
			TrajectoryPoint.VisualState = EBlackoutTrajectoryVisualState::Occluded;
		}
		else
		{
			TrajectoryPoint.VisualState = EBlackoutTrajectoryVisualState::Normal;
		}
	}

	CachedIndicatorData = RefreshedData;
	return true;
}

bool UBlackoutImpactIndicatorComponent::GetAimTargetHitResult(FHitResult& OutHitResult, FVector& OutTraceEnd) const
{
	OutHitResult = FHitResult();
	OutTraceEnd = FVector::ZeroVector;

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (OwnerController)
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
		OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		const FVector TraceStart = ViewLocation;
		OutTraceEnd = TraceStart + ViewRotation.Vector() * MaxTraceDistance;

		const UBlackoutCombatComponent* ResolvedCombatComponent = ResolveCombatComponent();
		const AActor* IgnoredWeapon = ResolvedCombatComponent ? ResolvedCombatComponent->GetEquippedWeapon() : nullptr;
		return PerformWeaponTrace(TraceStart, OutTraceEnd, IgnoredWeapon, OutHitResult);
	}

	if (const AActor* OwnerActor = GetOwner())
	{
		OutTraceEnd = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * MaxTraceDistance;
		return true;
	}

	return false;
}

FVector UBlackoutImpactIndicatorComponent::GetAimTargetPoint() const
{
	FHitResult HitResult;
	FVector TraceEnd = FVector::ZeroVector;
	if (GetAimTargetHitResult(HitResult, TraceEnd))
	{
		return HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd;
	}

	return FVector::ZeroVector;
}

bool UBlackoutImpactIndicatorComponent::GetTrueImpactPoint(FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd) const
{
	return GetTrueImpactPointInternal(OutHitResult, OutImpactPoint, OutTraceEnd, nullptr, nullptr);
}

bool UBlackoutImpactIndicatorComponent::GetTrueImpactPointInternal(
	FHitResult& OutHitResult,
	FVector& OutImpactPoint,
	FVector& OutTraceEnd,
	float* OutProjectileTravelDistance,
	TArray<FBlackoutTrajectoryPointData>* OutTrajectoryPoints) const
{
	OutHitResult = FHitResult();
	OutImpactPoint = FVector::ZeroVector;
	OutTraceEnd = FVector::ZeroVector;
	if (OutTrajectoryPoints)
	{
		OutTrajectoryPoints->Reset();
	}
	if (OutProjectileTravelDistance)
	{
		*OutProjectileTravelDistance = 0.0f;
	}

	const UBlackoutCombatComponent* ResolvedCombatComponent = ResolveCombatComponent();
	const ABOFirearm* Firearm = ResolvedCombatComponent ? ResolvedCombatComponent->GetEquippedFirearm() : nullptr;
	if (!Firearm)
	{
		return false;
	}

	if (Firearm->UsesHitscan())
	{
		return GetHitscanImpactHitResult(Firearm, OutHitResult, OutImpactPoint, OutTraceEnd);
	}

	return GetProjectileImpactHitResult(Firearm, OutHitResult, OutImpactPoint, OutTraceEnd, OutProjectileTravelDistance, OutTrajectoryPoints);
}

UBlackoutCombatComponent* UBlackoutImpactIndicatorComponent::ResolveCombatComponent() const
{
	if (UBlackoutCombatComponent* ExistingCombatComponent = CombatComponent.Get())
	{
		return ExistingCombatComponent;
	}

	return GetOwner() ? GetOwner()->FindComponentByClass<UBlackoutCombatComponent>() : nullptr;
}

bool UBlackoutImpactIndicatorComponent::BuildImpactIndicatorUpdateKey(FBlackoutImpactIndicatorUpdateKey& OutUpdateKey) const
{
	OutUpdateKey = FBlackoutImpactIndicatorUpdateKey();

	const UBlackoutCombatComponent* ResolvedCombatComponent = ResolveCombatComponent();
	if (!ResolvedCombatComponent)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (OwnerController)
	{
		FVector ViewLocation = FVector::ZeroVector;
		OwnerController->GetPlayerViewPoint(ViewLocation, OutUpdateKey.CameraRotation);
	}
	else if (const AActor* OwnerActor = GetOwner())
	{
		OutUpdateKey.CameraRotation = OwnerActor->GetActorRotation();
	}
	else
	{
		return false;
	}

	OutUpdateKey.bIsAiming = ResolvedCombatComponent->IsAiming();
	OutUpdateKey.bIsReloading = IsOwnerReloading();
	OutUpdateKey.EquippedFirearm = ResolvedCombatComponent->GetEquippedFirearm();
	if (const ABOFirearm* Firearm = OutUpdateKey.EquippedFirearm.Get())
	{
		// 발사 방향은 조준점과 총구 위치로 다시 산출하므로 캐시 키에는 총구 위치만 포함합니다.
		OutUpdateKey.MuzzleLocation = Firearm->GetMuzzleTransform().GetLocation();
	}

	return true;
}

bool UBlackoutImpactIndicatorComponent::HasImpactIndicatorUpdateInputChanged(const FBlackoutImpactIndicatorUpdateKey& UpdateKey) const
{
	if (!bHasCachedIndicatorData)
	{
		return true;
	}

	return LastUpdateKey.bIsAiming != UpdateKey.bIsAiming
		|| LastUpdateKey.bIsReloading != UpdateKey.bIsReloading
		|| LastUpdateKey.EquippedFirearm.Get() != UpdateKey.EquippedFirearm.Get()
		|| !LastUpdateKey.CameraRotation.Equals(UpdateKey.CameraRotation, ImpactIndicatorCameraRotationToleranceDegrees)
		|| !LastUpdateKey.MuzzleLocation.Equals(UpdateKey.MuzzleLocation, ImpactIndicatorMuzzleLocationTolerance);
}

bool UBlackoutImpactIndicatorComponent::GetHitscanImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd) const
{
	if (!Firearm)
	{
		return false;
	}

	const FVector MuzzleLocation = Firearm->GetMuzzleTransform().GetLocation();
	const FVector FireDirection = ResolveFireDirection(Firearm);
	if (FireDirection.IsNearlyZero())
	{
		return false;
	}

	OutTraceEnd = MuzzleLocation + FireDirection * MaxTraceDistance;
	if (!PerformWeaponTrace(MuzzleLocation, OutTraceEnd, Firearm, OutHitResult))
	{
		return false;
	}

	OutImpactPoint = OutHitResult.bBlockingHit ? OutHitResult.ImpactPoint : OutTraceEnd;
	return true;
}

bool UBlackoutImpactIndicatorComponent::GetProjectileImpactHitResult(
	const ABOFirearm* Firearm,
	FHitResult& OutHitResult,
	FVector& OutImpactPoint,
	FVector& OutTraceEnd,
	float* OutTravelDistance,
	TArray<FBlackoutTrajectoryPointData>* OutTrajectoryPoints) const
{
	OutHitResult = FHitResult();
	OutImpactPoint = FVector::ZeroVector;
	OutTraceEnd = FVector::ZeroVector;
	if (OutTrajectoryPoints)
	{
		OutTrajectoryPoints->Reset();
	}
	if (OutTravelDistance)
	{
		*OutTravelDistance = 0.0f;
	}

	if (!Firearm)
	{
		return false;
	}

	FPredictProjectilePathParams PathParams;
	if (!BuildProjectilePathParams(Firearm, ResolveFireDirection(Firearm), PathParams))
	{
		return false;
	}

	FPredictProjectilePathResult PathResult;
	if (!UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult))
	{
		if (PathResult.PathData.Num() <= 0)
		{
			return false;
		}
	}

	float TravelDistance = 0.0f;
	TArray<FBlackoutTrajectoryPointData> TrajectoryPoints;
	BuildTrajectoryPoints(PathResult, TrajectoryPoints, TravelDistance);
	if (OutTravelDistance)
	{
		*OutTravelDistance = TravelDistance;
	}
	if (OutTrajectoryPoints)
	{
		*OutTrajectoryPoints = MoveTemp(TrajectoryPoints);
	}

	// PredictProjectilePath는 첫 blocking hit 이후를 계산하지 않으므로 1차 궤적 표시에 그대로 사용합니다.
	OutHitResult = PathResult.HitResult;
	if (OutHitResult.bBlockingHit)
	{
		OutImpactPoint = OutHitResult.ImpactPoint;
		OutTraceEnd = OutHitResult.TraceEnd;
		if (OutTravelDistance && PathResult.PathData.Num() <= 1)
		{
			*OutTravelDistance = FVector::Dist(Firearm->GetMuzzleTransform().GetLocation(), OutImpactPoint);
		}
		return true;
	}

	if (PathResult.PathData.Num() > 0)
	{
		OutImpactPoint = PathResult.PathData.Last().Location;
		OutTraceEnd = OutImpactPoint;
		return true;
	}

	return false;
}

void UBlackoutImpactIndicatorComponent::BuildTrajectoryPoints(
	const FPredictProjectilePathResult& PathResult,
	TArray<FBlackoutTrajectoryPointData>& OutTrajectoryPoints,
	float& OutTravelDistance) const
{
	OutTrajectoryPoints.Reset();
	OutTravelDistance = 0.0f;

	if (PathResult.PathData.Num() <= 0)
	{
		return;
	}

	FVector PreviousLocation = PathResult.PathData[0].Location;
	FBlackoutTrajectoryPointData FirstPoint;
	FirstPoint.WorldLocation = PreviousLocation;
	FirstPoint.DistanceFromMuzzle = 0.0f;
	OutTrajectoryPoints.Add(FirstPoint);

	for (int32 PathIndex = 1; PathIndex < PathResult.PathData.Num(); ++PathIndex)
	{
		const FVector CurrentLocation = PathResult.PathData[PathIndex].Location;
		OutTravelDistance += FVector::Dist(PreviousLocation, CurrentLocation);

		FBlackoutTrajectoryPointData TrajectoryPoint;
		TrajectoryPoint.WorldLocation = CurrentLocation;
		TrajectoryPoint.DistanceFromMuzzle = OutTravelDistance;
		OutTrajectoryPoints.Add(TrajectoryPoint);

		PreviousLocation = CurrentLocation;
	}

	if (PathResult.HitResult.bBlockingHit && !PreviousLocation.Equals(PathResult.HitResult.ImpactPoint, KINDA_SMALL_NUMBER))
	{
		// 마지막 샘플이 실제 ImpactPoint와 떨어져 있으면 HUD 끝점 정렬을 위해 ImpactPoint를 추가합니다.
		OutTravelDistance += FVector::Dist(PreviousLocation, PathResult.HitResult.ImpactPoint);

		FBlackoutTrajectoryPointData ImpactPoint;
		ImpactPoint.WorldLocation = PathResult.HitResult.ImpactPoint;
		ImpactPoint.DistanceFromMuzzle = OutTravelDistance;
		OutTrajectoryPoints.Add(ImpactPoint);
	}
}

bool UBlackoutImpactIndicatorComponent::BuildProjectilePathParams(const ABOFirearm* Firearm, const FVector& LaunchDirection, FPredictProjectilePathParams& OutParams) const
{
	if (!Firearm || LaunchDirection.IsNearlyZero())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float LaunchSpeed = Firearm->GetProjectileLaunchSpeed();
	if (LaunchSpeed <= 0.0f)
	{
		return false;
	}

	OutParams = FPredictProjectilePathParams();
	OutParams.StartLocation = Firearm->GetMuzzleTransform().GetLocation();
	OutParams.LaunchVelocity = LaunchDirection * LaunchSpeed;
	OutParams.ProjectileRadius = FMath::Max(Firearm->GetProjectileCollisionRadius(), ProjectilePredictionRadiusFallback);
	OutParams.MaxSimTime = ProjectilePredictionTime;
	OutParams.SimFrequency = ProjectilePredictionSimFrequency;
	OutParams.bTraceWithCollision = true;
	OutParams.bTraceWithChannel = true;
	OutParams.TraceChannel = BlackoutCollisionChannels::WeaponTrace;
	OutParams.DrawDebugType = EDrawDebugTrace::None;
	OutParams.ActorsToIgnore.Add(GetOwner());
	OutParams.ActorsToIgnore.Add(const_cast<ABOFirearm*>(Firearm));
	AppendIgnoredProjectiles(World, OutParams.ActorsToIgnore);
	OutParams.OverrideGravityZ = World->GetGravityZ() * Firearm->GetProjectileGravityScale();
	return true;
}

bool UBlackoutImpactIndicatorComponent::PerformWeaponTrace(const FVector& TraceStart, const FVector& TraceEnd, const AActor* IgnoredActor, FHitResult& OutHitResult) const
{
	OutHitResult = FHitResult();

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutImpactIndicator_WeaponTrace), false, GetOwner());
	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}
	AddIgnoredProjectiles(World, QueryParams);

	World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams);
	return true;
}

bool UBlackoutImpactIndicatorComponent::IsProjectileImpactOccludedFromCamera(const ABOFirearm* Firearm, const FVector& ImpactPoint) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (!OwnerController || !Firearm)
	{
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const float ViewToImpactDistance = FVector::Dist(ViewLocation, ImpactPoint);
	if (ViewToImpactDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FHitResult CameraHitResult;
	if (!PerformWeaponTrace(ViewLocation, ImpactPoint, Firearm, CameraHitResult) || !CameraHitResult.bBlockingHit)
	{
		return false;
	}

	constexpr float ImpactSurfaceTolerance = 10.0f;
	return CameraHitResult.Distance < ViewToImpactDistance - ImpactSurfaceTolerance;
}

bool UBlackoutImpactIndicatorComponent::IsOwnerReloading() const
{
	const AActor* OwnerActor = GetOwner();
	UAbilitySystemComponent* AbilitySystemComponent = OwnerActor
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor)
		: nullptr;

	if (!AbilitySystemComponent)
	{
		const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
		const APlayerState* OwnerPlayerState = OwnerPawn ? OwnerPawn->GetPlayerState() : nullptr;
		AbilitySystemComponent = OwnerPlayerState
			? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPlayerState)
			: nullptr;
	}

	return AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Reloading);
}

AActor* UBlackoutImpactIndicatorComponent::ResolveTargetActor(const FHitResult& HitResult) const
{
	if (!HitResult.bBlockingHit)
	{
		return nullptr;
	}

	if (AActor* HitActor = HitResult.GetActor())
	{
		return HitActor;
	}

	const UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	return HitComponent ? HitComponent->GetOwner() : nullptr;
}

FVector UBlackoutImpactIndicatorComponent::ResolveFireDirection(const ABOFirearm* Firearm) const
{
	if (!Firearm)
	{
		return FVector::ZeroVector;
	}

	FVector FireDirection = (GetAimTargetPoint() - Firearm->GetMuzzleTransform().GetLocation()).GetSafeNormal();
	if (FireDirection.IsNearlyZero())
	{
		if (const AActor* OwnerActor = GetOwner())
		{
			FireDirection = OwnerActor->GetActorForwardVector();
		}
	}

	return FireDirection;
}
