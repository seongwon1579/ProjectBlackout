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

	// ToString() 은 "/Game/Path.AssetName" 을 반환하는데 ServerTravel 은 PackageName 만 허용.
	const FString PackageName = BattleMapPath.GetLongPackageName();
	BO_LOG_NET(Log, "StartBattle : ServerTravel -> %s", *PackageName);
	GetWorld()->ServerTravel(PackageName);
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
