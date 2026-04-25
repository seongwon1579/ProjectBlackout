// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutCheckpoint.h"
#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutLog.h"
#include "Core/BlackoutTypes.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"


// Sets default values
ABlackoutCheckpoint::ABlackoutCheckpoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InteractionPrompt = FText::FromString(TEXT("체크포인트에서 휴식"));
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
	
	ABlackoutBattleGameMode* BattleGameMode = GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>();
	if (!BattleGameMode)
	{
		return;
	}
	
	BattleGameMode->HandleCheckpoint(this);
	
	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		if (ABlackoutPlayerState* PS = InteractorPawn->GetPlayerState<ABlackoutPlayerState>())
		{
			PS->ApplyBattleTransitionPolicy(EBattleTransitionType::CheckpointRest);
		}
	}
	BO_LOG_NET(Log, "Checkpoint OnInteract: %s → CheckpointRest 본인 적용", *GetName());
}

FText ABlackoutCheckpoint::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

// Called when the game starts or when spawned
void ABlackoutCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !bAutoRegisterOnBeginPlay)
	{
		return;
	}
	if (ABlackoutBattleGameMode* BattleGameMode = GetWorld()->GetAuthGameMode<
		ABlackoutBattleGameMode>())
	{
		BattleGameMode->HandleCheckpoint(this);
	}
}


