#include "Environment/BOBreakablePillarActor.h"

#include "Characters/BORavagerBoss.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Core/BlackoutLog.h"
#include "Engine/World.h"
#include "Framework/BlackoutGameState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* GetBlackoutNetModeString(const ENetMode NetMode)
	{
		switch (NetMode)
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return TEXT("Client");
		default:
			return TEXT("Unknown");
		}
	}
}

ABOBreakablePillarActor::ABOBreakablePillarActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	// BP에서 ChildActorComponent를 붙일 공통 루트입니다.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Ravager 공격 히트박스가 직접 겹치는 전용 판정 박스입니다.
	PillarHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("PillarHitbox"));
	PillarHitbox->SetupAttachment(Root);
	PillarHitbox->SetBoxExtent(FVector(120.0f, 120.0f, 300.0f));
	PillarHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PillarHitbox->SetCollisionObjectType(ECC_Pawn);
	PillarHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Ravager 공격 히트박스가 현재 Pawn 타입을 사용하므로 동일 채널 오버랩으로 판정을 받습니다.
	PillarHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PillarHitbox->SetGenerateOverlapEvents(true);
	PillarHitbox->CanCharacterStepUpOn = ECB_No;
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

	if (HasAuthority() && bIsBroken)
	{
		ScheduleBrokenPieceHide();
	}

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
	DOREPLIFETIME(ABOBreakablePillarActor, bAreBrokenPiecesHidden);
}

FGameplayTag ABOBreakablePillarActor::GetHitPartTag(FName BoneName) const
{
	(void)BoneName;
	return FGameplayTag();
}

void ABOBreakablePillarActor::ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName)
{
	(void)BoneName;

	if (!HasAuthority())
	{
		return;
	}

	if (bIsBroken)
	{
		return;
	}

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		BO_LOG_CORE(Warning, "기둥이 유효하지 않은 피해 스펙을 받았습니다: %s", *GetName());
		return;
	}

	if (!IsDamageSpecFromRavager(SpecHandle))
	{
		return;
	}

	BO_LOG_CORE(Log,
		"기둥 파괴 조건 충족: Actor=%s EffectClass=%s NetMode=%s",
		*GetName(),
		*GetNameSafe(BreakDustEffectClass.Get()),
		GetBlackoutNetModeString(GetNetMode()));

	BreakPillar();
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
	GetWorldTimerManager().ClearTimer(HideBrokenPiecesTimerHandle);

	bIsBroken = true;
	bAreBrokenPiecesHidden = false;
	ApplyCurrentState();
	ApplyBreakImpulse();
	BO_LOG_CORE(Log,
		"기둥 파괴 더스트 멀티캐스트 호출: Actor=%s EffectClass=%s NetMode=%s",
		*GetName(),
		*GetNameSafe(BreakDustEffectClass.Get()),
		GetBlackoutNetModeString(GetNetMode()));
	Multicast_PlayBreakDustEffect();
	ScheduleBrokenPieceHide();
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
	GetWorldTimerManager().ClearTimer(HideBrokenPiecesTimerHandle);
	bAreBrokenPiecesHidden = false;

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

	// 패키징 실행에서는 ChildActor가 초기 월드 좌표를 잠깐 잘못 들고 오는 경우가 있어
	// 캐시 전에 각 컴포넌트 기준 위치로 한 번 다시 정렬합니다.
	SyncChildActorToComponent(WholeMesh);

	for (UChildActorComponent* PieceComponent : BreakPieces)
	{
		SyncChildActorToComponent(PieceComponent);
	}

	CacheInitialPieceTransforms();
}

void ABOBreakablePillarActor::OnRep_IsBroken()
{
	BO_LOG_CORE(Log,
		"OnRep_IsBroken 호출: Actor=%s bIsBroken=%s bAreBrokenPiecesHidden=%s NetMode=%s",
		*GetName(),
		bIsBroken ? TEXT("true") : TEXT("false"),
		bAreBrokenPiecesHidden ? TEXT("true") : TEXT("false"),
		GetBlackoutNetModeString(GetNetMode()));

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

void ABOBreakablePillarActor::OnRep_AreBrokenPiecesHidden()
{
	ApplyCurrentState();
}

void ABOBreakablePillarActor::Multicast_PlayBreakDustEffect_Implementation()
{
	BO_LOG_CORE(Log,
		"기둥 파괴 더스트 멀티캐스트 수신: Actor=%s EffectClass=%s NetMode=%s LocalRole=%d",
		*GetName(),
		*GetNameSafe(BreakDustEffectClass.Get()),
		GetBlackoutNetModeString(GetNetMode()),
		static_cast<int32>(GetLocalRole()));

	PlayBreakDustEffect();
}

void ABOBreakablePillarActor::ApplyCurrentState()
{
	if (IsValid(PillarHitbox))
	{
		PillarHitbox->SetCollisionEnabled(!bIsBroken ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		PillarHitbox->SetGenerateOverlapEvents(!bIsBroken);
	}

	ApplyWholeMeshState(!bIsBroken);

	const bool bShowBrokenPieces = bIsBroken && !bAreBrokenPiecesHidden;

	for (UChildActorComponent* PieceComponent : BreakPieces)
	{
		ApplyPieceState(PieceComponent, bShowBrokenPieces, bShowBrokenPieces);
	}
}

void ABOBreakablePillarActor::ScheduleBrokenPieceHide()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(HideBrokenPiecesTimerHandle);

	if (!bIsBroken || bAreBrokenPiecesHidden || BrokenPieceVisibleDuration <= 0.0f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		HideBrokenPiecesTimerHandle,
		this,
		&ABOBreakablePillarActor::HideBrokenPieces,
		BrokenPieceVisibleDuration,
		false
	);
}

void ABOBreakablePillarActor::HideBrokenPieces()
{
	if (!HasAuthority() || !bIsBroken || bAreBrokenPiecesHidden)
	{
		return;
	}

	bAreBrokenPiecesHidden = true;
	ApplyCurrentState();
	ForceNetUpdate();
}

void ABOBreakablePillarActor::ApplyWholeMeshState(bool bVisible)
{
	if (!IsValid(WholeMesh))
	{
		return;
	}

	SyncChildActorToComponent(WholeMesh);

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

	if (AActor* PieceChildActor = PieceComponent->GetChildActor	())
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
		const FTransform InitialTransform = IsValid(PieceComponent)
			? PieceComponent->GetRelativeTransform()
			: FTransform::Identity;

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

		if (PieceComponent->Mobility != EComponentMobility::Movable)
		{
			PieceComponent->SetMobility(EComponentMobility::Movable);
		}

		const FTransform RestoreRelativeTransform = InitialPieceTransforms.IsValidIndex(Index)
			? InitialPieceTransforms[Index]
			: PieceComponent->GetRelativeTransform();

		PieceComponent->SetRelativeTransform(RestoreRelativeTransform);
		SyncChildActorToComponent(PieceComponent);
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

void ABOBreakablePillarActor::PlayBreakDustEffect()
{
	if (!BreakDustEffectClass)
	{
		
		return;
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		
		return;
	}

	// 기둥 기준 오프셋을 월드 좌표로 변환해 더스트 BP를 자연스럽게 배치합니다.
	const FVector SpawnLocation = GetActorTransform().TransformPosition(BreakDustLocationOffset);
	const FRotator SpawnRotation = (GetActorRotation() + BreakDustRotationOffset).GetNormalized();

	

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedDustEffect = World->SpawnActor<AActor>(BreakDustEffectClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (!IsValid(SpawnedDustEffect))
	{
		
		return;
	}

	BO_LOG_CORE(Log,
		"기둥 파괴 더스트 스폰 성공: Actor=%s SpawnedEffect=%s",
		*GetName(),
		*GetNameSafe(SpawnedDustEffect));

	static const FName TriggerEffectFunctionNames[] =
	{
		FName(TEXT("Trigger Effect")),
		FName(TEXT("TriggerEffect"))
	};

	UFunction* TriggerEffectFunction = nullptr;
	FName ResolvedTriggerEffectFunctionName = NAME_None;

	for (const FName& CandidateFunctionName : TriggerEffectFunctionNames)
	{
		TriggerEffectFunction = SpawnedDustEffect->FindFunction(CandidateFunctionName);
		if (TriggerEffectFunction)
		{
			ResolvedTriggerEffectFunctionName = CandidateFunctionName;
			break;
		}
	}

	if (TriggerEffectFunction)
	{
		SpawnedDustEffect->ProcessEvent(TriggerEffectFunction, nullptr);
		BO_LOG_CORE(Log,
			"기둥 파괴 더스트 TriggerEffect 호출 성공: Actor=%s SpawnedEffect=%s Function=%s",
			*GetName(),
			*GetNameSafe(SpawnedDustEffect),
			*ResolvedTriggerEffectFunctionName.ToString());
	}
	else
	{
		BO_LOG_CORE(Warning, 
			"기둥 파괴 더스트 TriggerEffect 호출 실패: 함수가 없음 Actor=%s SpawnedEffect=%s TriedFunctions=%s,%s",
			*GetName(),
			*GetNameSafe(SpawnedDustEffect),
			*TriggerEffectFunctionNames[0].ToString(),
			*TriggerEffectFunctionNames[1].ToString());
	}

	if (BreakDustLifeSpan > 0.0f) 
	{
		SpawnedDustEffect->SetLifeSpan(BreakDustLifeSpan);
		BO_LOG_CORE(Log,
			"기둥 파괴 더스트 수명 설정: Actor=%s SpawnedEffect=%s LifeSpan=%.2f",
			*GetName(),
			*GetNameSafe(SpawnedDustEffect),
			BreakDustLifeSpan);
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

void ABOBreakablePillarActor::SyncChildActorToComponent(UChildActorComponent* ChildActorComponent) const
{
	if (!IsValid(ChildActorComponent))
	{
		return;
	}

	if (ChildActorComponent->Mobility != EComponentMobility::Movable)
	{
		ChildActorComponent->SetMobility(EComponentMobility::Movable);
	}

	AActor* ChildActor = ChildActorComponent->GetChildActor();
	if (!IsValid(ChildActor))
	{
		return;
	}

	if (USceneComponent* ChildRootComponent = ChildActor->GetRootComponent())
	{
		if (ChildRootComponent->Mobility != EComponentMobility::Movable)
		{
			ChildRootComponent->SetMobility(EComponentMobility::Movable);
		}
	}

	ChildActor->SetActorTransform(ChildActorComponent->GetComponentTransform(), false, nullptr, ETeleportType::ResetPhysics);
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

bool ABOBreakablePillarActor::IsDamageSpecFromRavager(const FGameplayEffectSpecHandle& SpecHandle) const
{
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return false;
	}

	const FGameplayEffectContextHandle& EffectContext = SpecHandle.Data->GetContext();
	TArray<AActor*, TInlineAllocator<3>> SourceCandidates;
	SourceCandidates.Add(EffectContext.GetEffectCauser());
	SourceCandidates.Add(EffectContext.GetInstigator());
	SourceCandidates.Add(EffectContext.GetOriginalInstigator());

	for (AActor* SourceCandidate : SourceCandidates)
	{
		if (IsValid(SourceCandidate) && SourceCandidate->IsA(ABORavagerBoss::StaticClass()))
		{
			return true;
		}
	}

	if (const UObject* SourceObject = EffectContext.GetSourceObject())
	{
		if (const AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			return SourceActor->IsA(ABORavagerBoss::StaticClass());
		}
	}

	return false;
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
