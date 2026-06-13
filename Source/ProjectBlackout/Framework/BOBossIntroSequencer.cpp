// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/BOBossIntroSequencer.h"

#include "BlackoutBattleGameMode.h"
#include "BlackoutBossAIController.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"

// Sets default values
ABOBossIntroSequencer::ABOBossIntroSequencer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ABOBossIntroSequencer::PlayBossIntro()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Does not have Authority"))
		return;
	}
	ULevelSequencePlayer* Player = BossCutsceneActor ? BossCutsceneActor->GetSequencePlayer() : nullptr;
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: do not exist cut scene, so do Combat instantly"))
		OnCutsceneTimerExpired();
		return;
	}
	Player->OnFinished.AddDynamic(this, &ABOBossIntroSequencer::OnCutsceneTimerExpired);
	
	Multicast_PlayerCutscene();
}

void ABOBossIntroSequencer::ActivateBossAI()
{
	if (!HasAuthority() || !IsValid(TargetBoss))
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Does not have Authority, Valid Target"))
		return;
	}
	
	if (APawn* BossPawn = Cast<APawn>(TargetBoss))
	{
		if (ABlackoutBossAIController* Controller = Cast<ABlackoutBossAIController>(BossPawn->GetController()))
		{
			UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Request a Combat to Controller"))
			Controller->StartCombat();
		}
	}
}

void ABOBossIntroSequencer::Multicast_PlayerCutscene_Implementation()
{
	if (BossCutsceneActor && BossCutsceneActor->GetSequencePlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Play a cut scene"))
		BossCutsceneActor->GetSequencePlayer()->Play();
	}
}

void ABOBossIntroSequencer::OnCutsceneTimerExpired()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Does not have Authority"))
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Elapsed the time of cutscene"))
	ActivateBossAI();
}

void ABOBossIntroSequencer::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (ABlackoutBattleGameMode* GM = Cast<ABlackoutBattleGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->RegisterCutsceneManager(this);
		}
	}
}

