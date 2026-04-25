#include "BlackoutGameMode.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

ABlackoutGameMode::ABlackoutGameMode()
{
	GameStateClass        = ABlackoutGameState::StaticClass();
	PlayerStateClass      = ABlackoutPlayerState::StaticClass();
	PlayerControllerClass = ABlackoutPlayerController::StaticClass();
}

void ABlackoutGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 매칭 API(Nest.js)가 데디 실행 시 ?SessionId=<UUID> 형태로 넘겨주는 식별자를 보관한다.
	MatchmakingSessionId = UGameplayStatics::ParseOption(Options, TEXT("SessionId"));
	BO_LOG_NET(Log, "InitGame: Map=%s SessionId=%s", *MapName, *MatchmakingSessionId);
}

void ABlackoutGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	BO_LOG_NET(Log, "Player logged in: %s", *GetNameSafe(NewPlayer));

	ConnectedPlayers.AddUnique(NewPlayer);
	OnPlayerJoined(NewPlayer);
}

void ABlackoutGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		ConnectedPlayers.Remove(PC);
	}
	BO_LOG_NET(Log, "Player logged out: %s (remaining=%d)", *GetNameSafe(Exiting), ConnectedPlayers.Num());

	OnPlayerLeft(Exiting);
}

void ABlackoutGameMode::HandlePartyWipe()
{
	BO_LOG_CORE(Log, "HandlePartyWipe triggered");
}

// Ready 집계 기본 구현. 정원 미달이거나 한 명이라도 bIsReady == false 면 false.
bool ABlackoutGameMode::AllPlayersReady() const
{
	if (ConnectedPlayers.Num() < MaxPlayers) { return false; }
	if (!GameState) { return false; }

	for (APlayerState* PS : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS);
		if (!BlackoutPS || !BlackoutPS->bIsReady) { return false; }
	}
	return true;
}

// Ready 상태 갱신 직후 호출. 성립 시 자식 GameMode 훅 실행.
void ABlackoutGameMode::NotifyReadyChanged()
{
	if (AllPlayersReady())
	{
		OnAllPlayersReady();
	}
}
