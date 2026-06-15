// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/BOBossIntroSequencer.h"

#include "BlackoutBattleGameMode.h"
#include "BlackoutBossAIController.h"
#include "BlackoutMusicSubsystem.h"
#include "Engine/GameInstance.h"
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
	if (Player)
	{
		Player->OnFinished.AddDynamic(this, &ABOBossIntroSequencer::OnCutsceneTimerExpired);
	}
	
	Multicast_PlayerCutscene();

	// 컷신 액터가 없더라도 인트로 음악은 재생하고 AI 활성화만 즉시 진행합니다.
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: do not exist cut scene, so do Combat instantly"))
		OnCutsceneTimerExpired();
	}
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
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMusicSubsystem* MusicSubsystem = GameInstance->GetSubsystem<UBlackoutMusicSubsystem>())
		{
			// 보스 종류 하드코딩 대신 시퀀서 인스턴스에 지정된 음악을 그대로 재생합니다.
			MusicSubsystem->PlayMusicAsset(IntroMusic);
		}
	}

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
			// 전투 시작 타이밍은 BattleGameMode가 소유하고, 시퀀서는 자기 자신을 거기에 등록만 합니다.
			GM->RegisterCutsceneManager(this);
		}
	}
}

