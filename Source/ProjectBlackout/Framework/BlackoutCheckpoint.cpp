#include "BlackoutCheckpoint.h"

#include "BlackoutPlayerController.h"
#include "GameFramework/Pawn.h"

ABlackoutCheckpoint::ABlackoutCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InteractionPrompt = FText::FromString(TEXT("캐릭터 변경"));
}

bool ABlackoutCheckpoint::CanInteract_Implementation(AActor* Interactor) const
{
	return Interactor != nullptr;
}

void ABlackoutCheckpoint::OnInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}
	
	const APawn* InteractorPawn = Cast<APawn>(Interactor);
	if (!InteractorPawn)
	{
		return;
	}

	// 캐릭터 변경 UI 호출 — 상호작용한 클라에만 전송.
	if (ABlackoutPlayerController* PC = Cast<ABlackoutPlayerController>(InteractorPawn->GetController()))
	{
		PC->Client_OpenClassSelectUI();
	}
}

FText ABlackoutCheckpoint::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}
