#include "BlackoutLobbyGameMode.h"
#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"
#include "BlackoutLog.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"

bool ABlackoutLobbyGameMode::AllPlayersReady() const
{
	
	if (ConnectedPlayers.Num() < MaxPlayers)
	{
		return false;
	}
	if (!GameState)
	{
		return false;
	}
	
	for (APlayerState* PS : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS);
		if (!BlackoutPS || !BlackoutPS->bIsReady)
		{
			return false;
		}
	}
	return true;
}

void ABlackoutLobbyGameMode::StartBattle()
{
	if (!BattleMapPath.IsValid())
	{
		BO_LOG_NET(Error,"StartBattle 실패 : BattleMapPath 미설정 (BP_BlackoutLobbyGameMode에서 지정 필요)");
		return ;
	}
	BO_LOG_NET(Log, "StartBattle : ServerTravel -> %s",*BattleMapPath.ToString());
	GetWorld()->ServerTravel(BattleMapPath.ToString());
}

void ABlackoutLobbyGameMode::NotifyReadyChanged()
{
	if (AllPlayersReady())
	{
		StartBattle();
	}
}

void ABlackoutLobbyGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	if (ABlackoutPlayerController* BlackoutPC = Cast<ABlackoutPlayerController>(NewPlayer))
	{
		BlackoutPC->Client_OpenClassSelectUI();
	}
}
