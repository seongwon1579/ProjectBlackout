#include "BlackoutLobbyGameMode.h"
#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "BlackoutMatchFlowSubsystem.h"
#include "Data/BOCharacterRoster.h"
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
	BO_LOG_NET(Log, "StartBattle : stage %d ServerTravel -> %s (bUseSeamlessTravel=%d)",
		StageIndex, *PackageName, bUseSeamlessTravel ? 1 : 0);
	GetWorld()->ServerTravel(PackageName);

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
