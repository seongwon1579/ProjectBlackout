#include "BlackoutAreaGate.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameMode.h"
#include "BlackoutLog.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

ABlackoutAreaGate::ABlackoutAreaGate()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Barrier = CreateDefaultSubobject<UBoxComponent>(TEXT("Barrier"));
	SetRootComponent(Barrier);
	Barrier->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	InteractionPrompt = FText::FromString(TEXT("준비"));
}

void ABlackoutAreaGate::BeginPlay()
{
	Super::BeginPlay();

	// 매치 상태 변경 구독 — 서버/클라 양쪽 동일 파생으로 개폐. 초기 상태 즉시 반영.
	if (ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr)
	{
		GS->OnMatchStateChanged.AddDynamic(this, &ABlackoutAreaGate::HandleMatchStateChanged);
		ApplyOpenState(GS->CurrentMatchState == TargetCombatPhase);
	}
}

void ABlackoutAreaGate::HandleMatchStateChanged(EBlackoutMatchState NewState)
{
	ApplyOpenState(NewState == TargetCombatPhase);
}

EBlackoutMatchState ABlackoutAreaGate::GetPrecedingShelterPhase() const
{
	return TargetCombatPhase == EBlackoutMatchState::MainBossCombat
		? EBlackoutMatchState::ShelterMid
		: EBlackoutMatchState::ShelterPrep;
}

void ABlackoutAreaGate::ApplyOpenState(bool bOpen)
{
	bIsOpen = bOpen;
	if (Barrier)
	{
		Barrier->SetCollisionEnabled(bOpen
			? ECollisionEnabled::NoCollision
			: ECollisionEnabled::QueryAndPhysics);
	}
	BO_LOG_NET(Log, "AreaGate %s: %s", *GetName(), bOpen ? TEXT("Open") : TEXT("Closed"));
}

bool ABlackoutAreaGate::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor || bIsOpen)
	{
		return false;
	}
	const ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	return GS && GS->CurrentMatchState == GetPrecedingShelterPhase();
}

void ABlackoutAreaGate::OnInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !CanInteract_Implementation(Interactor))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	ABlackoutPlayerState* PS = Pawn ? Pawn->GetPlayerState<ABlackoutPlayerState>() : nullptr;
	if (!PS)
	{
		return;
	}

	PS->bIsReady = !PS->bIsReady;
	BO_LOG_NET(Log, "AreaGate Ready 토글: %s = %d", *GetNameSafe(PS), PS->bIsReady ? 1 : 0);

	if (ABlackoutGameMode* GM = GetWorld()->GetAuthGameMode<ABlackoutGameMode>())
	{
		GM->NotifyReadyChanged();
	}
}

FText ABlackoutAreaGate::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}
