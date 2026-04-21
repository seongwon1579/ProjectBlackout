#include "BlackoutGameMode.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutLog.h"
#include "Kismet/GameplayStatics.h"

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
