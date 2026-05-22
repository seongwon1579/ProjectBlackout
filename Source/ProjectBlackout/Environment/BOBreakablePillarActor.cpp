#include "Environment/BOBreakablePillarActor.h"

#include "Components/ChildActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Core/BlackoutLog.h"
#include "Framework/BlackoutGameState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ABOBreakablePillarActor::ABOBreakablePillarActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	// BP에서 ChildActorComponent를 붙일 공통 루트입니다.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ABOBreakablePillarActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshBreakPieces();

	if (!IsValid(WholeMesh))
	{
		BO_LOG_CORE(Warning, "WholeMesh가 비어 있습니다: %s", *GetName());
	}

	if (BreakPieces.IsEmpty())
	{
		BO_LOG_CORE(Warning, "브레이크 조각이 없습니다: %s", *GetName());
	}

	// 서버는 GameState 기록을 우선해 초기 상태를 맞춥니다.
	if (HasAuthority() && IsMarkedDestroyedInGameState())
	{
		bIsBroken = true;
	}

	if (!bIsBroken)
	{
		RestorePieceTransforms();
	}

	ApplyCurrentState();

	// 테스트가 필요할 때만 자동으로 BreakPillar를 호출합니다.
	if (HasAuthority() && !bIsBroken && bAutoBreakForTest)
	{
		GetWorldTimerManager().SetTimer(AutoBreakTimerHandle, this, &ABOBreakablePillarActor::BreakPillar, AutoBreakDelay, false);
	}
}

void ABOBreakablePillarActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABOBreakablePillarActor, bIsBroken);
}

void ABOBreakablePillarActor::BreakPillar()
{
	if (!HasAuthority())
	{
		BO_LOG_CORE(Warning, "BreakPillar는 서버 권한에서만 호출되어야 합니다: %s", *GetName());
		return;
	}

	if (bIsBroken)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoBreakTimerHandle);

	bIsBroken = true;
	ApplyCurrentState();
	ApplyBreakImpulse();
	UpdateDestroyedPillarState(true);
	ForceNetUpdate();

	BO_LOG_CORE(Log, "기둥이 파괴되었습니다. PillarId=%d, Actor=%s", PillarId, *GetName());
}

void ABOBreakablePillarActor::ResetPillar()
{
	if (!HasAuthority())
	{
		BO_LOG_CORE(Warning, "ResetPillar는 서버 권한에서만 호출되어야 합니다: %s", *GetName());
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoBreakTimerHandle);

	if (!bIsBroken)
	{
		RestorePieceTransforms();
		ApplyCurrentState();
		return;
	}

	bIsBroken = false;
	RestorePieceTransforms();
	ApplyCurrentState();
	UpdateDestroyedPillarState(false);
	ForceNetUpdate();

	BO_LOG_CORE(Log, "기둥이 초기 상태로 복원되었습니다. PillarId=%d, Actor=%s", PillarId, *GetName());
}

void ABOBreakablePillarActor::RefreshBreakPieces()
{
	TInlineComponentArray<UChildActorComponent*> FoundComponents;
	GetComponents(FoundComponents);

	// WholeMesh를 직접 지정하지 않았다면 이름으로 한 번 자동 탐색합니다.
	if (!IsValid(WholeMesh) && !WholeMeshComponentName.IsNone())
	{
		const FString WholeMeshNameString = WholeMeshComponentName.ToString();

		for (UChildActorComponent* FoundComponent : FoundComponents)
		{
			if (!IsValid(FoundComponent))
			{
				continue;
			}

			if (FoundComponent->GetFName() == WholeMeshComponentName || FoundComponent->GetName().Contains(WholeMeshNameString))
			{
				WholeMesh = FoundComponent;
				break;
			}
		}
	}

	if (bAutoCollectBreakPieces)
	{
		BreakPieces.Reset();

		for (UChildActorComponent* FoundComponent : FoundComponents)
		{
			if (!IsValid(FoundComponent) || FoundComponent == WholeMesh)
			{
				continue;
			}

			BreakPieces.AddUnique(FoundComponent);
		}
	}

	BreakPieces.RemoveAll([](const TObjectPtr<UChildActorComponent>& PieceComponent)
	{
		return !IsValid(PieceComponent);
	});

	CacheInitialPieceTransforms();
}

void ABOBreakablePillarActor::OnRep_IsBroken()
{
	if (!bIsBroken)
	{
		RestorePieceTransforms();
	}

	ApplyCurrentState();

	if (bIsBroken)
	{
		ApplyBreakImpulse();
	}
}

void ABOBreakablePillarActor::ApplyCurrentState()
{
	ApplyWholeMeshState(!bIsBroken);

	for (UChildActorComponent* PieceComponent : BreakPieces)
	{
		ApplyPieceState(PieceComponent, bIsBroken, bIsBroken);
	}
}

void ABOBreakablePillarActor::ApplyWholeMeshState(bool bVisible)
{
	if (!IsValid(WholeMesh))
	{
		return;
	}

	WholeMesh->SetHiddenInGame(!bVisible, true);
	WholeMesh->SetVisibility(bVisible, true);

	if (AActor* WholeChildActor = WholeMesh->GetChildActor())
	{
		WholeChildActor->SetActorHiddenInGame(!bVisible);
	}

	if (UPrimitiveComponent* PrimitiveComponent = ResolvePrimitiveFromChild(WholeMesh))
	{
		EnsurePrimitiveIsMovable(PrimitiveComponent);
		PrimitiveComponent->SetHiddenInGame(!bVisible, true);
		PrimitiveComponent->SetVisibility(bVisible, true);
		PrimitiveComponent->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetSimulatePhysics(false);
	}
}

void ABOBreakablePillarActor::ApplyPieceState(UChildActorComponent* PieceComponent, bool bVisible, bool bEnablePhysics)
{
	if (!IsValid(PieceComponent))
	{
		return;
	}

	PieceComponent->SetHiddenInGame(!bVisible, true);
	PieceComponent->SetVisibility(bVisible, true);

	if (AActor* PieceChildActor = PieceComponent->GetChildActor())
	{
		PieceChildActor->SetActorHiddenInGame(!bVisible);
	}

	if (UPrimitiveComponent* PrimitiveComponent = ResolvePrimitiveFromChild(PieceComponent))
	{
		EnsurePrimitiveIsMovable(PrimitiveComponent);
		PrimitiveComponent->SetHiddenInGame(!bVisible, true);
		PrimitiveComponent->SetVisibility(bVisible, true);
		PrimitiveComponent->SetLinearDamping(PieceLinearDamping);
		PrimitiveComponent->SetAngularDamping(PieceAngularDamping);
		PrimitiveComponent->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		// 부서진 조각은 플레이어와 적 Pawn을 막지 않고 그대로 통과시킵니다.
		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		PrimitiveComponent->SetSimulatePhysics(bEnablePhysics);

		if (!bEnablePhysics)
		{
			PrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
			PrimitiveComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false, NAME_None);
		}
		else
		{
			PrimitiveComponent->WakeAllRigidBodies();
		}
	}
}

void ABOBreakablePillarActor::CacheInitialPieceTransforms()
{
	InitialPieceTransforms.Reset();
	InitialPieceTransforms.Reserve(BreakPieces.Num());

	for (UChildActorComponent* PieceComponent : BreakPieces)
	{
		FTransform InitialTransform = FTransform::Identity;

		if (IsValid(PieceComponent))
		{
			if (AActor* PieceChildActor = PieceComponent->GetChildActor())
			{
				InitialTransform = PieceChildActor->GetActorTransform();
			}
			else
			{
				InitialTransform = PieceComponent->GetComponentTransform();
			}
		}

		InitialPieceTransforms.Add(InitialTransform);
	}
}

void ABOBreakablePillarActor::RestorePieceTransforms()
{
	for (int32 Index = 0; Index < BreakPieces.Num(); ++Index)
	{
		UChildActorComponent* PieceComponent = BreakPieces[Index];
		if (!IsValid(PieceComponent))
		{
			continue;
		}

		if (UPrimitiveComponent* PrimitiveComponent = ResolvePrimitiveFromChild(PieceComponent))
		{
			EnsurePrimitiveIsMovable(PrimitiveComponent);
			PrimitiveComponent->SetSimulatePhysics(false);
			PrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
			PrimitiveComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false, NAME_None);
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AActor* PieceChildActor = PieceComponent->GetChildActor())
		{
			const FTransform RestoreTransform = InitialPieceTransforms.IsValidIndex(Index)
				? InitialPieceTransforms[Index]
				: PieceComponent->GetComponentTransform();

			PieceChildActor->SetActorTransform(RestoreTransform, false, nullptr, ETeleportType::ResetPhysics);
		}
	}
}

void ABOBreakablePillarActor::ApplyBreakImpulse()
{
	if (BreakImpulseStrength <= 0.0f && BreakImpulseUpwardBoost <= 0.0f)
	{
		return;
	}

	for (UChildActorComponent* PieceComponent : BreakPieces)
	{
		UPrimitiveComponent* PrimitiveComponent = ResolvePrimitiveFromChild(PieceComponent);
		if (!IsValid(PrimitiveComponent) || !PrimitiveComponent->IsSimulatingPhysics())
		{
			continue;
		}

		FVector OutwardDirection = (PrimitiveComponent->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
		if (OutwardDirection.IsNearlyZero())
		{
			OutwardDirection = GetActorForwardVector();
		}

		const FVector Impulse = (OutwardDirection * BreakImpulseStrength) + (GetActorUpVector() * BreakImpulseUpwardBoost);
		PrimitiveComponent->AddImpulse(Impulse, NAME_None, true);
	}
}

UPrimitiveComponent* ABOBreakablePillarActor::ResolvePrimitiveFromChild(UChildActorComponent* ChildActorComponent) const
{
	if (!IsValid(ChildActorComponent))
	{
		return nullptr;
	}

	AActor* ChildActor = ChildActorComponent->GetChildActor();
	if (!IsValid(ChildActor))
	{
		return nullptr;
	}

	return Cast<UPrimitiveComponent>(ChildActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
}

void ABOBreakablePillarActor::EnsurePrimitiveIsMovable(UPrimitiveComponent* PrimitiveComponent) const
{
	if (!IsValid(PrimitiveComponent))
	{
		return;
	}

	if (PrimitiveComponent->Mobility != EComponentMobility::Movable)
	{
		PrimitiveComponent->SetMobility(EComponentMobility::Movable);
	}

	if (USceneComponent* AttachParent = PrimitiveComponent->GetAttachParent())
	{
		if (AttachParent->Mobility != EComponentMobility::Movable)
		{
			AttachParent->SetMobility(EComponentMobility::Movable);
		}
	}

	if (AActor* OwnerActor = PrimitiveComponent->GetOwner())
	{
		if (USceneComponent* RootSceneComponent = OwnerActor->GetRootComponent())
		{
			if (RootSceneComponent->Mobility != EComponentMobility::Movable)
			{
				RootSceneComponent->SetMobility(EComponentMobility::Movable);
			}
		}
	}
}

void ABOBreakablePillarActor::UpdateDestroyedPillarState(bool bDestroyed) const
{
	if (!HasAuthority() || PillarId == INDEX_NONE)
	{
		return;
	}

	ABlackoutGameState* BlackoutGameState = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	if (!IsValid(BlackoutGameState))
	{
		BO_LOG_CORE(Warning, "GameState를 찾지 못해 파괴 상태를 기록하지 못했습니다: %s", *GetName());
		return;
	}

	if (bDestroyed)
	{
		BlackoutGameState->DestroyedPillarIds.AddUnique(PillarId);
	}
	else
	{
		BlackoutGameState->DestroyedPillarIds.Remove(PillarId);
	}
}

bool ABOBreakablePillarActor::IsMarkedDestroyedInGameState() const
{
	if (PillarId == INDEX_NONE)
	{
		return false;
	}

	const ABlackoutGameState* BlackoutGameState = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	if (!IsValid(BlackoutGameState))
	{
		return false;
	}

	return BlackoutGameState->DestroyedPillarIds.Contains(PillarId);
}
