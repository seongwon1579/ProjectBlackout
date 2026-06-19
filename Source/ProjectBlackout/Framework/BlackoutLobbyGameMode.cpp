#include "BlackoutLobbyGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "BlackoutMatchFlowSubsystem.h"
#include "Data/BOCharacterRoster.h"
#include "GameFramework/GameModeBase.h"
#include  "Characters/BlackoutPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BlackoutDedicatedSessionSubsystem.h"

void ABlackoutLobbyGameMode::StartBattle()
{

	if (bTravelInitiated)
	{
		BO_LOG_NET(Warning , "StartBattle 중복 호출 무시 - ServerTravel 이미 진행 중");
		return;
	}
	
	const UBlackoutMatchFlowSubsystem* FlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBlackoutMatchFlowSubsystem>() : nullptr;
	const int32 StageIndex = FlowSubsystem ? FlowSubsystem->GetCurrentStageIndex() : 0;
	
	if (!BossStageMapPaths.IsValidIndex(StageIndex) || !BossStageMapPaths[StageIndex].IsValid())
	{
		BO_LOG_NET(Error, "StartBattle 실패: BossStageMapPaths[%d] 미설정 (BP_BlackoutLobbyGameMode 확인)", StageIndex);
		return;
	}
	
	bTravelInitiated = true;

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::Starting);

		// 로비에서 부여한 무한 치트가 보스전으로 넘어가지 않도록 travel 직전 전원 해제.
		// (CopyProperties 가 seamless travel 시 치트 플래그를 새 PS 로 복사하므로 여기서 차단)
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS))
			{
				BlackoutPS->SetDebugCheatFlags(false, false, false);
			}
		}
	}
	
	// 일반 이동 = 흰색 페이드 -> 대기후 travel
	BroadcastScreenFadeOut(FLinearColor::White ,true);
	GetWorldTimerManager().SetTimer(FadeTravelTimerHandle ,this ,&ABlackoutLobbyGameMode::DoStartBattleTravel , FadeOutTravelDelay , false);
}

// 전원 Ready 성립 시 전투 맵으로 ServerTravel 트리거.
void ABlackoutLobbyGameMode::OnAllPlayersReady()
{
	StartBattle();
}

void ABlackoutLobbyGameMode::OnSeamlessArrival(APlayerController* PC)
{
	HandleLobbyArrival(PC);
}

void ABlackoutLobbyGameMode::BO_ForceStartBattle()
{
	BO_LOG_NET(Warning, "[테스트] BO_ForceStartBattle — 정원/Ready 무시 강제 ServerTravel");
	StartBattle();
}

void ABlackoutLobbyGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	HandleLobbyArrival(NewPlayer);
}



void ABlackoutLobbyGameMode::HandleLobbyArrival(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(PC->GetPawn()))
	{
		PlayerCharacter->RestoreToFullState();
	}

	// 로비는 안전한 샌드박스 — 탄/체력/스테미나 무한 적용. 보스전 travel 직전 StartBattle 에서 해제.
	if (ABlackoutPlayerState* PS = PC->GetPlayerState<ABlackoutPlayerState>())
	{
		PS->SetDebugCheatFlags(true, true, true);
	}

	if (ConnectedPlayers.Num() == MaxPlayers)
	{
		if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
		{
			if (GS->CurrentMatchState == EBlackoutMatchState::WaitingForPlayers)
			{
				TransitionTo(EBlackoutMatchState::ShelterPrep);
			}
		}
	}
}

void ABlackoutLobbyGameMode::HandleEmptyServerReset()
{
	// 로비에서 전원 퇴장 — 매칭 서버에 finish/idle 보고해야 markIdle 됨(안 하면 status='playing' 좀비).
	// /finish 는 세션·플레이어키 정리(세션 살아있을 때), /idle 는 serverId 로 status 직접 복구(세션 만료에도).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBlackoutDedicatedSessionSubsystem* Dedi =
				GI->GetSubsystem<UBlackoutDedicatedSessionSubsystem>())
		{
			Dedi->ReportFinishToMatchmakingServer();
			Dedi->ReportIdleToMatchmakingServer();
		}
	}

	// 다음 파티 위해 깨끗한 상태로 reload (InitGame 이 MatchState/스테이지 초기화). 빈 서버라 비용 무관.
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("?restart"));
	}
}

void ABlackoutLobbyGameMode::DoStartBattleTravel()
{
	const UBlackoutMatchFlowSubsystem* FlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBlackoutMatchFlowSubsystem>() : nullptr;
	const int32 StageIndex = FlowSubsystem ? FlowSubsystem->GetCurrentStageIndex() : 0;
	if (!BossStageMapPaths.IsValidIndex(StageIndex)) { return; }


	const FString PackageName = BossStageMapPaths[StageIndex].GetLongPackageName();
	BO_LOG_NET(Log, "StartBattle: stage %d ServerTravel -> %s", StageIndex, *PackageName);
	GetWorld()->ServerTravel(PackageName);
}

UClass* ABlackoutLobbyGameMode::GetDefaultPawnClassForController_Implementation(
	AController* InController)
{
	const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	const UBOCharacterRoster* Roster = GS ? GS->CharacterRoster : nullptr;
	if (Roster && InController)
	{
		if (const ABlackoutPlayerState* PS = InController->GetPlayerState<ABlackoutPlayerState>())
		{
			if (PS->SelectedClassTag.IsValid())
			{
				if (TSubclassOf<APawn> SelectedClass = Roster->FindPawnClassByTag(PS->SelectedClassTag))
				{
					return SelectedClass.Get();
				}
			}
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
