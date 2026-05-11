#include "Combat/Components/BlackoutImpactIndicatorComponent.h"

#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Components/PrimitiveComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

namespace
{
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
}

bool UBlackoutImpactIndicatorComponent::GetImpactIndicatorData(FBlackoutImpactIndicatorData& OutIndicatorData) const
{
	OutIndicatorData = FBlackoutImpactIndicatorData();

	const UBlackoutCombatComponent* ResolvedCombatComponent = ResolveCombatComponent();

	// 조준 여부와 무관하게 현재 스프레드는 항상 전달
	OutIndicatorData.SpreadNormalized = ResolvedCombatComponent ? ResolvedCombatComponent->GetNormalizedSpread() : 0.0f;

	if (!ResolvedCombatComponent || !ResolvedCombatComponent->IsAiming())
	{
		return false;
	}

	const ABOFirearm* Firearm = ResolvedCombatComponent->GetEquippedFirearm();
	if (!Firearm)
	{
		return false;
	}

	FHitResult AimHitResult;
	FVector AimTraceEnd = FVector::ZeroVector;
	if (!GetAimTargetHitResult(AimHitResult, AimTraceEnd))
	{
		return false;
	}

	FHitResult TrueImpactHitResult;
	FVector ImpactPoint = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	float ProjectileTravelDistance = 0.0f;
	if (!GetTrueImpactPointInternal(TrueImpactHitResult, ImpactPoint, TraceEnd, &ProjectileTravelDistance))
	{
		return false;
	}

	const bool bUsesProjectilePrediction = !Firearm->UsesHitscan();
	const float DistanceFromMuzzle = FVector::Dist(Firearm->GetMuzzleTransform().GetLocation(), ImpactPoint);
	const float ProjectileImpactFuseArmDistance = Firearm->GetProjectileImpactFuseArmDistance();

	OutIndicatorData.bIsVisible = true;
	OutIndicatorData.bHasBlockingHit = TrueImpactHitResult.bBlockingHit;
	OutIndicatorData.bUsesProjectilePrediction = bUsesProjectilePrediction;
	OutIndicatorData.bProjectileImpactFuseInactive = bUsesProjectilePrediction
		&& TrueImpactHitResult.bBlockingHit
		&& ProjectileImpactFuseArmDistance > 0.0f
		&& ProjectileTravelDistance < ProjectileImpactFuseArmDistance;
	OutIndicatorData.bTargetMismatch = !bUsesProjectilePrediction
		&& AimHitResult.bBlockingHit
		&& ResolveTargetActor(AimHitResult) != ResolveTargetActor(TrueImpactHitResult);
	OutIndicatorData.bIsOccludedFromCamera = bUsesProjectilePrediction
		&& IsProjectileImpactOccludedFromCamera(Firearm, ImpactPoint);
	OutIndicatorData.WorldLocation = ImpactPoint;
	OutIndicatorData.TraceEndLocation = TraceEnd;
	OutIndicatorData.DistanceFromMuzzle = DistanceFromMuzzle;

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
	return GetTrueImpactPointInternal(OutHitResult, OutImpactPoint, OutTraceEnd, nullptr);
}

bool UBlackoutImpactIndicatorComponent::GetTrueImpactPointInternal(FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutProjectileTravelDistance) const
{
	OutHitResult = FHitResult();
	OutImpactPoint = FVector::ZeroVector;
	OutTraceEnd = FVector::ZeroVector;
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

	return GetProjectileImpactHitResult(Firearm, OutHitResult, OutImpactPoint, OutTraceEnd, OutProjectileTravelDistance);
}

UBlackoutCombatComponent* UBlackoutImpactIndicatorComponent::ResolveCombatComponent() const
{
	if (UBlackoutCombatComponent* ExistingCombatComponent = CombatComponent.Get())
	{
		return ExistingCombatComponent;
	}

	return GetOwner() ? GetOwner()->FindComponentByClass<UBlackoutCombatComponent>() : nullptr;
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

bool UBlackoutImpactIndicatorComponent::GetProjectileImpactHitResult(const ABOFirearm* Firearm, FHitResult& OutHitResult, FVector& OutImpactPoint, FVector& OutTraceEnd, float* OutTravelDistance) const
{
	OutHitResult = FHitResult();
	OutImpactPoint = FVector::ZeroVector;
	OutTraceEnd = FVector::ZeroVector;
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

	if (OutTravelDistance && PathResult.PathData.Num() > 0)
	{
		float TravelDistance = 0.0f;
		for (int32 PathIndex = 1; PathIndex < PathResult.PathData.Num(); ++PathIndex)
		{
			TravelDistance += FVector::Dist(
				PathResult.PathData[PathIndex - 1].Location,
				PathResult.PathData[PathIndex].Location);
		}
		if (PathResult.HitResult.bBlockingHit)
		{
			TravelDistance += FVector::Dist(PathResult.PathData.Last().Location, PathResult.HitResult.ImpactPoint);
		}
		*OutTravelDistance = TravelDistance;
	}

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
