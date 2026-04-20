#include "BlackoutGameMode.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutLog.h"

ABlackoutGameMode::ABlackoutGameMode()
{
	GameStateClass        = ABlackoutGameState::StaticClass();
	PlayerStateClass      = ABlackoutPlayerState::StaticClass();
	PlayerControllerClass = ABlackoutPlayerController::StaticClass();
}

void ABlackoutGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	BO_LOG_NET(Log, "Player logged in: %s", *GetNameSafe(NewPlayer));
}

void ABlackoutGameMode::HandlePartyWipe()
{
	BO_LOG_CORE(Log, "HandlePartyWipe triggered");
}
