#include "BlackoutGameState.h"
#include "Net/UnrealNetwork.h"
#include "BlackoutLog.h"

ABlackoutGameState::ABlackoutGameState()
{
}

void ABlackoutGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutGameState, CurrentMatchState);
	DOREPLIFETIME(ABlackoutGameState, MatchTimer);
	DOREPLIFETIME(ABlackoutGameState, DestroyedPillarIds);
	DOREPLIFETIME(ABlackoutGameState, bRedMistPhaseActive);
}

void ABlackoutGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerArrayChanged.Broadcast();
}

void ABlackoutGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnPlayerArrayChanged.Broadcast();
}

// 서버 Authority 전용. 클라 호출 차단 + 동일 상태 중복 전환 무시.
void ABlackoutGameState::SetMatchState(EBlackoutMatchState NewState)
{
	if (!HasAuthority()) { return; }
	if (CurrentMatchState == NewState) { return; }

	CurrentMatchState = NewState;
	BO_LOG_NET(Log, "MatchState 전환: %s", *UEnum::GetValueAsString(NewState));
}

void ABlackoutGameState::OnRep_CurrentMatchState()
{
	BO_LOG_NET(Log, "MatchState OnRep: %s", *UEnum::GetValueAsString(CurrentMatchState));
}

void ABlackoutGameState::OnRep_DestroyedPillarIds()
{
	BO_LOG_NET(Verbose, "DestroyedPillarIds updated, count=%d", DestroyedPillarIds.Num());
}
