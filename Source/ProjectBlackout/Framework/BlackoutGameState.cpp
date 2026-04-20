#include "BlackoutGameState.h"
#include "Net/UnrealNetwork.h"
#include "BlackoutLog.h"

ABlackoutGameState::ABlackoutGameState()
{
}

void ABlackoutGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutGameState, MatchTimer);
	DOREPLIFETIME(ABlackoutGameState, DestroyedPillarIds);
	DOREPLIFETIME(ABlackoutGameState, bRedMistPhaseActive);
}

void ABlackoutGameState::OnRep_DestroyedPillarIds()
{
	BO_LOG_NET(Verbose, "DestroyedPillarIds updated, count=%d", DestroyedPillarIds.Num());
}
