// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/BOBossIntroSequencer.h"

#include "BlackoutBossAIController.h"
#include "BlackoutMusicSubsystem.h"
#include "Engine/GameInstance.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"
#include "UI/BlackoutHUD.h"
#include "UI/BlackoutHUDWidget.h"

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
	if (BossCutsceneActor && BossCutsceneActor->GetSequencePlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Play a cut scene"))
		BossCutsceneActor->GetSequencePlayer()->Play();
	}
}

void ABOBossIntroSequencer::Multicast_PlayIntroMusic_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMusicSubsystem* MusicSubsystem = GameInstance->GetSubsystem<UBlackoutMusicSubsystem>())
		{
			MusicSubsystem->PlayMusicAsset(IntroMusic);
		}
	}
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ABlackoutHUD* HUD = PC->GetHUD<ABlackoutHUD>())
		{
			if (UBlackoutHUDWidget* HUDWidget = HUD->GetHUDWidget())
			{
				HUDWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		
		PC->SetCinematicMode(true, false, true, true, true);
		PC->DisableInput(PC);
	}

	if (BossCutsceneActor && BossCutsceneActor->GetSequencePlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Play a cut scene"))
		BossCutsceneActor->GetSequencePlayer()->Play();
	}
}

void ABOBossIntroSequencer::Multicast_EndCutscene_Implementation()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ABlackoutHUD* HUD = PC->GetHUD<ABlackoutHUD>())
		{
			if (UBlackoutHUDWidget* HUDWidget = HUD->GetHUDWidget())
			{
				HUDWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
		
		PC->SetCinematicMode(false, false, false, true, true);
		PC->EnableInput(PC);
	}
}

void ABOBossIntroSequencer::OnCutsceneTimerExpired()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Does not have Authority"))
		return;
	}
	Multicast_EndCutscene();
	
	UE_LOG(LogTemp, Warning, TEXT("ABossCutsceneManager: Elapsed the time of cutscene"))
	ActivateBossAI();
}

void ABOBossIntroSequencer::BeginPlay()
{
	Super::BeginPlay();
	if (bIsTestMode)
	{
		PlayBossIntro();
	}
}

