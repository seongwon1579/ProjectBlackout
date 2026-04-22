#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"

// 플레이어 접속 직후 전투 진입 자원 정책 적용. 정원 충족 시 InCombatReady 로 전환 (Ready 대기 단계).
void ABlackoutBattleGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	if (!NewPlayer) { return; }

	if (ABlackoutPlayerState* PS = NewPlayer->GetPlayerState<ABlackoutPlayerState>())
	{
		PS->ApplyBattleTransitionPolicy(EBattleTransitionType::LobbyToBattle);
	}

	// 4인 정원 충족 시 매치 상태를 InCombatReady 로 한 번만 전환.
	if (ConnectedPlayers.Num() == MaxPlayers)
	{
		if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
		{
			if (GS->CurrentMatchState != EBlackoutMatchState::InCombatReady)
			{
				GS->SetMatchState(EBlackoutMatchState::InCombatReady);
			}
		}
	}
}

// 전원 Ready 시 InCombat 전환 + 보스 활성화 훅. 실제 보스 활성 로직은 전투팀 합류 시 연결.
void ABlackoutBattleGameMode::OnAllPlayersReady()
{
	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::InCombat);
	}
	BO_LOG_NET(Log, "전원 Ready — InCombat 진입. 보스 활성화 훅 호출 대상");
}

// 화톳불 상호작용 시 호출되어 현재 체크포인트 액터 갱신.
void ABlackoutBattleGameMode::HandleCheckpoint(AActor* BonfireActor)
{
	if (!BonfireActor) { return; }

	CurrentCheckpointActor = BonfireActor;
	BO_LOG_NET(Log, "체크포인트 갱신: %s", *BonfireActor->GetName());
}

// 파티 전멸 감지 시 호출. 전원 Ready 리셋 + InCombatReady 복귀. 실제 텔레포트·보스 상태 리셋은 향후 보강.
void ABlackoutBattleGameMode::HandlePartyWipe()
{
	Super::HandlePartyWipe();

	// 전원 bIsReady 리셋 — 체크포인트 방에서 다시 Ready 받기 위함.
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS))
			{
				BlackoutPS->bIsReady = false;
			}
		}
	}

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::InCombatReady);
	}

	const FString Location = CurrentCheckpointActor
		? CurrentCheckpointActor->GetName()
		: TEXT("none");
	BO_LOG_NET(Log, "파티 전멸 - 체크포인트 복귀 대기 + Ready 재요청 (CurrentCheckpoint=%s)", *Location);
}
