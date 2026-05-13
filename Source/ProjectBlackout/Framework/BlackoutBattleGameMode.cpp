#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"

#include "BlackoutDedicatedSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include  "Engine/GameInstance.h"

UClass* ABlackoutBattleGameMode::
GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (PlayerClassPool.Num() == 0)
	{
		BO_LOG_NET(Warning, "PlayerClassPool 비어있음 — DefaultPawnClass fallback");
		return Super::GetDefaultPawnClassForController_Implementation(
			InController);
	}
	
	const int32 ClassIndex = NextPlayerClassIndex & PlayerClassPool.Num();
	const TSubclassOf<APawn> SelectedPawn = PlayerClassPool[ClassIndex];
	++NextPlayerClassIndex;
	
	BO_LOG_NET(Log, "플레이어 #%d 클래스 분배 - %s",
	NextPlayerClassIndex, *GetNameSafe(SelectedPawn.Get()));

	return SelectedPawn.Get();
}

// 플레이어 접속 직후 전투 진입 자원 정책 적용. 정원 충족 시 InCombatReady 로 전환 (Ready 대기 단계).
void ABlackoutBattleGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	if (ABlackoutPlayerState* PS = NewPlayer->GetPlayerState<
		ABlackoutPlayerState>())
	{
		PS->ApplyBattleTransitionPolicy(EBattleTransitionType::LobbyToBattle);
	}

	// InCombat/Ended 이후는 재접속으로 되돌리지 않음. InLobby/Starting 에서만 InCombatReady 진입 허용.
	// ServerTravel 직후 Battle GameState 는 InLobby 로 새로 생성됨 — Lobby 의 Starting 값은 승계 안됨.
	if (ConnectedPlayers.Num() == MaxPlayers)
	{
		if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
		{
			if (GS->CurrentMatchState == EBlackoutMatchState::InLobby ||
				GS->CurrentMatchState == EBlackoutMatchState::Starting)
			{
				if (bAutoStartOnFull)
				{
					GS->SetMatchState(EBlackoutMatchState::InCombat);
					BO_LOG_NET(Log, "정원 충족 - 자동 InCombat (시연 모드)");
				}else
				{
				GS->SetMatchState(EBlackoutMatchState::InCombatReady);
				}
			}
		}
	}
}

void ABlackoutBattleGameMode::OnPlayerLeft(AController* Exiting)
{
	if (ConnectedPlayers.Num() == 0)
	{
		EndMatch(EBlackoutMatchEndReason::AllPlayersLeft);
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
	if (!BonfireActor)
	{
		return;
	}

	CurrentCheckpointActor = BonfireActor;
	BO_LOG_NET(Log, "체크포인트 갱신: %s", *BonfireActor->GetName());
}

void ABlackoutBattleGameMode::InitGame(const FString& MapName,
                                       const FString& Options,
                                       FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// ClientTravel URL 옵션 "?SessionId=session" 추출
	const FString SessionId = UGameplayStatics::ParseOption(
		Options,TEXT("SessionId"));
	if (SessionId.IsEmpty())
	{
		BO_LOG_NET(Warning,
		           "BattleGameMode InitGame - SessionId 옵션 X (Options=%s)",
		           *Options);
		return;
	}

	// Subsystem 보관
	UGameInstance* GameInstance = GetGameInstance();
	UBlackoutDedicatedSessionSubsystem* DedicatedSessionSubsystem = GameInstance
		? GameInstance->GetSubsystem<UBlackoutDedicatedSessionSubsystem>()
		: nullptr;

	if (!DedicatedSessionSubsystem)
	{
		BO_LOG_NET(Error, "DedicatedSessionSubsystem 못 찾음");
		return;
	}

	DedicatedSessionSubsystem->SetSessionId(SessionId);
	BO_LOG_NET(Log, "BattleGameMode InitGame -SessionId=%s 보관", *SessionId);
}

void ABlackoutBattleGameMode::EndMatch(EBlackoutMatchEndReason Reason)
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();

	if (!GS)
	{
		return;
	}
	if (GS->CurrentMatchState == EBlackoutMatchState::Ended)
	{
		return;
	}

	GS->SetMatchState(EBlackoutMatchState::Ended);

	const TCHAR* ReasonText = TEXT("Unknown");
	switch (Reason)
	{
	case EBlackoutMatchEndReason::BossDefeated: ReasonText = TEXT(
			"BossDefeated");
		break;
	case EBlackoutMatchEndReason::AllPlayersLeft: ReasonText = TEXT(
			"AllPlayersLeft");
		break;
	case EBlackoutMatchEndReason::Timeout: ReasonText = TEXT("Timeout");
		break;
	}
	BO_LOG_NET(Log, "EndMatch: Reason=%s", ReasonText)

	// 매칭 서버에 종료 보고 - 데디 측에서만 Http 호출
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutDedicatedSessionSubsystem* DedicatedSessionSubsystem =
			GameInstance->GetSubsystem<UBlackoutDedicatedSessionSubsystem>())
		{
			DedicatedSessionSubsystem->ReportFinishToMatchmakingServer();
		}
	}
}

// 파티 전멸 감지 시 호출. 체크포인트 텔레포트 + PartyWipeRestart 정책 + Ready 리셋 + InCombatReady 복귀.
void ABlackoutBattleGameMode::HandlePartyWipe()
{
	Super::HandlePartyWipe();

	if (!GameState)
	{
		return;
	}

	const bool bHasCheckpoint = CurrentCheckpointActor != nullptr;
	const FVector RespawnLocation = bHasCheckpoint
		                                ? CurrentCheckpointActor->
		                                GetActorLocation()
		                                : FVector::ZeroVector;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS);
		if (!BlackoutPS)
		{
			continue;
		}

		BlackoutPS->bIsReady = false;
		BlackoutPS->ApplyBattleTransitionPolicy(
			EBattleTransitionType::PartyWipeRestart);

		if (!bHasCheckpoint)
		{
			continue;
		}

		if (APlayerController* PC = BlackoutPS->GetPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				Pawn->SetActorLocation(RespawnLocation, false, nullptr,
				                       ETeleportType::TeleportPhysics);
			}
		}
	}

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::InCombatReady);
	}

	const FString Location = bHasCheckpoint
		                         ? CurrentCheckpointActor->GetName()
		                         : TEXT("none");
	BO_LOG_NET(Log, "파티 전멸 - 체크포인트 복귀 + Ready 재요청 (CurrentCheckpoint=%s)",
	           *Location);
}
