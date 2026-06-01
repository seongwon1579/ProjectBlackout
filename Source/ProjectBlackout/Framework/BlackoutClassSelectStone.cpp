// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutClassSelectStone.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

// Sets default values
ABlackoutClassSelectStone::ABlackoutClassSelectStone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates =true;
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionPrompt = FText::FromString(TEXT("캐릭터 선택"));
}

bool ABlackoutClassSelectStone::CanInteract_Implementation(
	AActor* Interactor) const
{
	if (!Interactor) return false;
	const ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	return GS && GS->CurrentMatchState == EBlackoutMatchState::ShelterPrep;
}

void ABlackoutClassSelectStone::OnInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !CanInteract_Implementation(Interactor)) return;
	APawn* Pawn = Cast<APawn>(Interactor);
	ABlackoutPlayerController* PC = Pawn ? Cast<ABlackoutPlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) return;
	//  PC 클라에 UI (호스트는 Client RPC skip 되므로 직접 호출).
	if (PC->IsLocalController())
	{
		PC->Client_OpenClassSelectUI_Implementation();
	}
	else
	{
		PC->Client_OpenClassSelectUI();
	}
}

FText ABlackoutClassSelectStone::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}



