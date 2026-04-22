#include "BlackoutLobbyGameMode.h"
#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"

void ABlackoutLobbyGameMode::StartBattle()
{
	if (!BattleMapPath.IsValid())
	{
		BO_LOG_NET(Error, "StartBattle 실패 : BattleMapPath 미설정 (BP_BlackoutLobbyGameMode에서 지정 필요)");
		return;
	}

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::Starting);
	}

	BO_LOG_NET(Log, "StartBattle : ServerTravel -> %s", *BattleMapPath.ToString());
	GetWorld()->ServerTravel(BattleMapPath.ToString());
}

// 전원 Ready 성립 시 전투 맵으로 ServerTravel 트리거.
void ABlackoutLobbyGameMode::OnAllPlayersReady()
{
	StartBattle();
}

void ABlackoutLobbyGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	if (ABlackoutPlayerController* BlackoutPC = Cast<ABlackoutPlayerController>(NewPlayer))
	{
		BlackoutPC->Client_OpenClassSelectUI();
	}
}
