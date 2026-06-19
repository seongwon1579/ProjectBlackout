#include "BlackoutGameState.h"
#include "Net/UnrealNetwork.h"
#include "BlackoutLog.h"
#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"

ABlackoutGameState::ABlackoutGameState()
{
}

void ABlackoutGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutGameState, CurrentMatchState);
	DOREPLIFETIME(ABlackoutGameState, MatchTimer);
	DOREPLIFETIME(ABlackoutGameState, DestroyedPillarIds);
	DOREPLIFETIME(ABlackoutGameState, bIsMatchResultVisible);
	DOREPLIFETIME(ABlackoutGameState, DefeatedBossType);
	DOREPLIFETIME(ABlackoutGameState, MatchResultVisibleServerTime);
	DOREPLIFETIME(ABlackoutGameState, MatchResultAutoTravelServerTime);
	DOREPLIFETIME(ABlackoutGameState, MatchResultParticipants);
	DOREPLIFETIME(ABlackoutGameState, bRedMistPhaseActive);
	DOREPLIFETIME(ABlackoutGameState, bIsSurrenderVoteActive);
	DOREPLIFETIME(ABlackoutGameState, SurrenderVoteYesCount);
	DOREPLIFETIME(ABlackoutGameState, SurrenderVoteNoCount);
	DOREPLIFETIME(ABlackoutGameState, RequiredSurrenderVoteCount);
	DOREPLIFETIME(ABlackoutGameState, SurrenderVoteEndTimeSeconds);
	DOREPLIFETIME(ABlackoutGameState, SurrenderVoteCooldownEndTime);
	DOREPLIFETIME(ABlackoutGameState, bMatchSummaryReady);
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

void ABlackoutGameState::MarkMatchSummaryReady()
{
	if (!HasAuthority() || bMatchSummaryReady)
	{
		return;
	}
	
	bMatchSummaryReady = true;
	OnMatchSummaryReady.Broadcast();
}

// 서버 Authority 전용. 클라 호출 차단 + 동일 상태 중복 전환 무시.
void ABlackoutGameState::SetMatchState(EBlackoutMatchState NewState)
{
	if (!HasAuthority()) { return; }
	if (CurrentMatchState == NewState) { return; }

	CurrentMatchState = NewState;
	BO_LOG_NET(Log, "MatchState 전환: %s", *UEnum::GetValueAsString(NewState));
	OnMatchStateChanged.Broadcast(NewState);
	TryOpenLocalClassSelectUI();  // 서버(host) 로컬 PC
}

void ABlackoutGameState::OnRep_CurrentMatchState()
{
	BO_LOG_NET(Log, "MatchState OnRep: %s", *UEnum::GetValueAsString(CurrentMatchState));
	OnMatchStateChanged.Broadcast(CurrentMatchState);
	TryOpenLocalClassSelectUI();  // 클라 로컬 PC
}

void ABlackoutGameState::OnRep_DestroyedPillarIds()
{
	BO_LOG_NET(Verbose, "DestroyedPillarIds updated, count=%d", DestroyedPillarIds.Num());
}

void ABlackoutGameState::SetMatchResultState(
	bool bNewVisible,
	EBossType NewDefeatedBossType,
	float VisibleServerTime,
	float AutoTravelServerTime,
	const TArray<ABlackoutPlayerState*>& Participants)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsMatchResultVisible = bNewVisible;
	DefeatedBossType = NewDefeatedBossType;
	MatchResultVisibleServerTime = VisibleServerTime;
	MatchResultAutoTravelServerTime = AutoTravelServerTime;

	MatchResultParticipants.Reset();
	MatchResultParticipants.Reserve(Participants.Num());
	for (ABlackoutPlayerState* Participant : Participants)
	{
		if (Participant)
		{
			MatchResultParticipants.Add(Participant);
		}
	}

	BO_LOG_NET(Log,
	           "결과창 상태 갱신: Visible=%s Boss=%s Participants=%d AutoTravelAt=%.2f",
	           bIsMatchResultVisible ? TEXT("true") : TEXT("false"),
	           *UEnum::GetValueAsString(DefeatedBossType),
	           MatchResultParticipants.Num(),
	           MatchResultAutoTravelServerTime);

	OnMatchResultStateChanged.Broadcast();
}

void ABlackoutGameState::OnRep_MatchResultState()
{
	OnMatchResultStateChanged.Broadcast();
}

void ABlackoutGameState::OnRep_SurrenderVoteActive()
{
	OnSurrenderVoteStateChanged.Broadcast(
		bIsSurrenderVoteActive,
		SurrenderVoteYesCount,
		SurrenderVoteNoCount,
		SurrenderVoteEndTimeSeconds
	);
}

void ABlackoutGameState::OnRep_MatchSummaryReady()
{
	if (bMatchSummaryReady)
	{
		OnMatchSummaryReady.Broadcast();
	}
}

void ABlackoutGameState::TryOpenLocalClassSelectUI()
{
	if (CurrentMatchState != EBlackoutMatchState::ShelterPrep)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (ABlackoutPlayerController* PC =
			Cast<ABlackoutPlayerController>(World->GetFirstPlayerController()))
		{
			if (const ABlackoutPlayerState* PS = PC->GetPlayerState<ABlackoutPlayerState>())
			{
				if (PS->SelectedClassTag.IsValid())
				{
					return;
				}
			}
			PC->Client_OpenClassSelectUI_Implementation();
		}
	}
}
