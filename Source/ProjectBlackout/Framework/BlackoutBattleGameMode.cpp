#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"

#include "BlackoutDedicatedSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include  "Engine/GameInstance.h"

namespace
{
	bool IsActiveCombatState(EBlackoutMatchState MatchState)
	{
		return MatchState == EBlackoutMatchState::InCombat
			|| MatchState == EBlackoutMatchState::MidBossCombat
			|| MatchState == EBlackoutMatchState::MainBossCombat;
	}
}

UClass* ABlackoutBattleGameMode::
GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (PlayerClassPool.Num() == 0)
	{
		BO_LOG_NET(Warning, "PlayerClassPool 비어있음 — DefaultPawnClass fallback");
		return Super::GetDefaultPawnClassForController_Implementation(
			InController);
	}

	// 같은 컨트롤러 재요청 시 캐시 반환 — 엔진이 한 로그인당 본 함수를 여러 번 호출하므로 카운터 1회만 증가.
	if (const TSubclassOf<APawn>* Cached = ControllerToClass.Find(InController))
	{
		return Cached->Get();
	}

	const int32 ClassIndex = NextPlayerClassIndex % PlayerClassPool.Num();
	const TSubclassOf<APawn> SelectedPawn = PlayerClassPool[ClassIndex];
	ControllerToClass.Add(InController, SelectedPawn);
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

	// 정원 충족 → 시작 쉘터(ShelterPrep) 진입 + 클래스선택 UI. 단일맵 직행(ServerTravel 없음).
	if (ConnectedPlayers.Num() == MaxPlayers)
	{
		if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
		{
			if (GS->CurrentMatchState == EBlackoutMatchState::WaitingForPlayers)
			{
				TransitionTo(EBlackoutMatchState::ShelterPrep);

				for (const TObjectPtr<APlayerController>& PC : ConnectedPlayers)
				{
					if (ABlackoutPlayerController* BPC = Cast<ABlackoutPlayerController>(PC))
					{
						BPC->Client_OpenClassSelectUI();
					}
				}

				if (bAutoStartOnFull)
				{
					BO_LOG_NET(Warning,
						"[테스트] bAutoStartOnFull — 클래스선택/Ready 생략. 출시 빌드면 비활성 필요");
					for (APlayerState* PS : GameState->PlayerArray)
					{
						if (ABlackoutPlayerState* BPS = Cast<ABlackoutPlayerState>(PS))
						{
							BPS->bIsReady = true;
						}
					}
					NotifyReadyChanged();  // 정상 Ready 경로로 합류
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
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		return;
	}

	switch (GS->CurrentMatchState)
	{
	case EBlackoutMatchState::ShelterPrep:
		TransitionTo(EBlackoutMatchState::MidBossCombat);
		break;
	case EBlackoutMatchState::ShelterMid:
		TransitionTo(EBlackoutMatchState::MainBossCombat);
		break;
	default:
		return;  // 쉘터 페이즈가 아니면 무시
	}

	// 게이트 개폐와 보스 활성은 매치 상태 전이를 구독하는 측(게이트/보스)에서 처리한다.
	BO_LOG_NET(Log, "전원 Ready — 보스 전투 전이 + 게이트 Open 대상");
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

void ABlackoutBattleGameMode::RegisterArena(
	TScriptInterface<IBlackoutArenaResettableInterface> Arena)
{
	CurrentArena = Arena;
	BO_LOG_NET(Log, "아레나 등록: %s", *GetNameSafe(Arena.GetObject()));
}

void ABlackoutBattleGameMode::OnMidBossDefeated()
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS || GS->CurrentMatchState != EBlackoutMatchState::MidBossCombat)
	{
		return;
	}

	TransitionTo(EBlackoutMatchState::ShelterMid);
	// 중간 거점 쉘터 활성/게이트 잠금은 매치 상태 구독 측에서 처리된다.
	BO_LOG_NET(Log, "중간 보스 처치 — 중간 거점 쉘터로 전이");
}

void ABlackoutBattleGameMode::BO_SimMidBossDefeated()
{
	BO_LOG_NET(Warning, "[테스트] BO_SimMidBossDefeated — 중간 보스 처치 시뮬레이션");
	OnMidBossDefeated();
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

void ABlackoutBattleGameMode::InitGameState()
{
	Super::InitGameState();

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::WaitingForPlayers);  // 초기 상태(전이표 비대상)
	}
}

void ABlackoutBattleGameMode::PreLogin(const FString& Options, const FString& Address,
	const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return;  // 상위에서 이미 거부
	}

	if (const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		if (GS->CurrentMatchState != EBlackoutMatchState::WaitingForPlayers)
		{
			// ErrorMessage 가 비어있지 않으면 엔진이 접속을 거부한다.
			ErrorMessage = TEXT("Match already in progress");
			BO_LOG_NET(Warning, "PreLogin 거부 — 매치 진행 중 (state=%s)",
				*UEnum::GetValueAsString(GS->CurrentMatchState));
		}
	}
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

void ABlackoutBattleGameMode::NotifyPlayerFullyDead(ABlackoutPlayerCharacter* DeadPlayer)
{
	if (!DeadPlayer)
	{
		BO_LOG_NET(Warning, "완전 사망 알림 무시: DeadPlayer가 비어 있음");
		return;
	}

	BO_LOG_NET(Log, "플레이어 완전 사망 감지: %s", *GetNameSafe(DeadPlayer));
	RefreshSpectatorsForDeadTarget(DeadPlayer);

	if (ABlackoutPlayerController* DeadPlayerController = Cast<ABlackoutPlayerController>(DeadPlayer->GetController()))
	{
		AssignSpectateTargetForDeadPlayer(DeadPlayerController);
	}
	else
	{
		BO_LOG_NET(Warning, "관전 전환 실패: 사망 플레이어 컨트롤러를 찾을 수 없음 Player=%s", *GetNameSafe(DeadPlayer));
	}

	EvaluatePartyWipe();
}

void ABlackoutBattleGameMode::EvaluatePartyWipe()
{
	ABlackoutGameState* BlackoutGameState = GetGameState<ABlackoutGameState>();
	if (!BlackoutGameState)
	{
		BO_LOG_NET(Error, "전멸 평가 실패: GameState가 비어 있음");
		return;
	}

	if (!IsActiveCombatState(BlackoutGameState->CurrentMatchState))
	{
		BO_LOG_NET(Verbose, "전멸 평가 스킵: 전투 진행 상태가 아님 State=%d",
			static_cast<int32>(BlackoutGameState->CurrentMatchState));
		return;
	}

	int32 AlivePlayers = 0;
	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		APlayerController* PlayerController = BlackoutPlayerState->GetPlayerController();
		const ABlackoutPlayerCharacter* PlayerCharacter = PlayerController
			? Cast<ABlackoutPlayerCharacter>(PlayerController->GetPawn())
			: nullptr;

		if (PlayerCharacter && !PlayerCharacter->IsDead())
		{
			++AlivePlayers;
		}
	}

	BO_LOG_NET(Log, "전멸 평가: AlivePlayers=%d TotalPlayers=%d",
		AlivePlayers,
		BlackoutGameState->PlayerArray.Num());

	if (AlivePlayers <= 0)
	{
		HandlePartyWipe();
	}
}

ABlackoutPlayerCharacter* ABlackoutBattleGameMode::FindInitialSpectateTarget(
	ABlackoutPlayerController* SpectatorController)
{
	ABlackoutGameState* BlackoutGameState = GetGameState<ABlackoutGameState>();
	if (!BlackoutGameState)
	{
		BO_LOG_NET(Error, "관전 대상 탐색 실패: GameState가 비어 있음");
		return nullptr;
	}

	const ABlackoutPlayerCharacter* SpectatorPawn = SpectatorController
		? Cast<ABlackoutPlayerCharacter>(SpectatorController->GetPawn())
		: nullptr;

	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		APlayerController* CandidateController = BlackoutPlayerState->GetPlayerController();
		ABlackoutPlayerCharacter* Candidate = CandidateController
			? Cast<ABlackoutPlayerCharacter>(CandidateController->GetPawn())
			: nullptr;

		if (!Candidate || Candidate == SpectatorPawn || Candidate->IsDead())
		{
			continue;
		}

		return Candidate;
	}

	return nullptr;
}

void ABlackoutBattleGameMode::AssignSpectateTargetForDeadPlayer(ABlackoutPlayerController* SpectatorController)
{
	if (!SpectatorController)
	{
		BO_LOG_NET(Warning, "관전 대상 지정 실패: SpectatorController가 비어 있음");
		return;
	}

	ABlackoutPlayerCharacter* SpectateTarget = FindInitialSpectateTarget(SpectatorController);
	if (!SpectateTarget)
	{
		BO_LOG_NET(Log, "관전 대상 없음: Controller=%s", *GetNameSafe(SpectatorController));
		return;
	}

	SpectatorController->EnterSpectatorMode();
	SpectatorController->SetViewTargetWithBlend(SpectateTarget, 0.35f);
	SpectatorController->Client_SetSpectateTarget(SpectateTarget, 0.35f);

	BO_LOG_NET(Log,
		"관전 대상 지정: Spectator=%s Target=%s Downed=%s",
		*GetNameSafe(SpectatorController),
		*GetNameSafe(SpectateTarget),
		SpectateTarget->IsDowned() ? TEXT("true") : TEXT("false"));
}

void ABlackoutBattleGameMode::RefreshSpectatorsForDeadTarget(ABlackoutPlayerCharacter* DeadTarget)
{
	if (!DeadTarget || !GameState)
	{
		return;
	}

	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		ABlackoutPlayerController* SpectatorController = Cast<ABlackoutPlayerController>(
			BlackoutPlayerState->GetPlayerController());
		const ABlackoutPlayerCharacter* SpectatorPawn = SpectatorController
			? Cast<ABlackoutPlayerCharacter>(SpectatorController->GetPawn())
			: nullptr;

		if (!SpectatorController || !SpectatorPawn || !SpectatorPawn->IsDead())
		{
			continue;
		}

		if (SpectatorController->GetViewTarget() == DeadTarget)
		{
			AssignSpectateTargetForDeadPlayer(SpectatorController);
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

		if (APlayerController* PC = BlackoutPS->GetPlayerController())
		{
			if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(PC->GetPawn()))
			{
				PlayerCharacter->RestoreFromPartyWipeRestart();
			}

			if (ABlackoutPlayerController* BlackoutPC = Cast<ABlackoutPlayerController>(PC))
			{
				BlackoutPC->ExitSpectatorMode();
				BlackoutPC->Client_ReturnToOwnPawnView(0.15f);
			}
		}

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

	if (CurrentArena)
	{
		IBlackoutArenaResettableInterface::Execute_ResetArena(CurrentArena.GetObject());
	}

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		switch (GS->CurrentMatchState)
		{
		case EBlackoutMatchState::MidBossCombat:
			TransitionTo(EBlackoutMatchState::ShelterPrep);
			break;
		case EBlackoutMatchState::MainBossCombat:
			TransitionTo(EBlackoutMatchState::ShelterMid);
			break;
		default:
			break;
		}
	}

	const FString Location = bHasCheckpoint
		                         ? CurrentCheckpointActor->GetName()
		                         : TEXT("none");
	BO_LOG_NET(Log, "파티 전멸 - 체크포인트 복귀 + Ready 재요청 (CurrentCheckpoint=%s)",
	           *Location);
}
