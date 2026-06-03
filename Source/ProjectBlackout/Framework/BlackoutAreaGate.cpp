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
	Barrier->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionPrompt = FText::FromString(TEXT("준비"));
}

bool ABlackoutAreaGate::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}
	const ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	return GS && GS->CurrentMatchState == EBlackoutMatchState::ShelterPrep;
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

	PS->SetReadyState(!PS->IsReady());
	BO_LOG_NET(Log, "이동 비석 Ready 토글: %s = %d", *GetNameSafe(PS), PS->IsReady() ? 1 : 0);

	if (ABlackoutGameMode* GM = GetWorld()->GetAuthGameMode<ABlackoutGameMode>())
	{
		GM->NotifyReadyChanged();
	}
}

FText ABlackoutAreaGate::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

