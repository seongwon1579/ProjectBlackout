#include "BlackoutLobbyGameMode.h"
#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "BlackoutMatchFlowSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include  "Characters/BlackoutPlayerCharacter.h"

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
	}

	const FString PackageName = BossStageMapPaths[StageIndex].GetLongPackageName();
	BO_LOG_NET(Log, "StartBattle : stage %d ServerTravel -> %s", StageIndex, *PackageName);
	GetWorld()->ServerTravel(PackageName);

}

// 전원 Ready 성립 시 전투 맵으로 ServerTravel 트리거.
void ABlackoutLobbyGameMode::OnAllPlayersReady()
{
	StartBattle();
}

void ABlackoutLobbyGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	
	if (NewPlayer)
	{
		if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(NewPlayer->GetPawn()))
		{
			PlayerCharacter->RestoreToFullState();
		}
	}
	
	
	if (ConnectedPlayers.Num() ==  MaxPlayers)
	{
		ABlackoutGameState* GS =GetGameState<ABlackoutGameState>();
		if (GS && GS->CurrentMatchState == EBlackoutMatchState::WaitingForPlayers)
		{
			// UI 노출은 GameState OnRep(TryOpenLocalClassSelectUI)이 로컬 PC 마다 처리.
			TransitionTo(EBlackoutMatchState::ShelterPrep);
		}
	}
	
}
