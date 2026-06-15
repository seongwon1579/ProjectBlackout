#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Data/BOCharacterRoster.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "BlackoutDedicatedSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include  "Engine/GameInstance.h"
#include "BlackoutMatchFlowSubsystem.h"
#include "Characters/BlackoutBossCharacter.h"
#include "EngineUtils.h"
#include "BlackoutTelemetrySampler.h"
#include "BOBossIntroSequencer.h"

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

	// SelectedClassTag 우선
	const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (const UBOCharacterRoster* CharacterRoster = GS
		                                                ? GS->CharacterRoster
		                                                : nullptr)
	{
		if (const ABlackoutPlayerState* PS = InController
			                                     ? InController->GetPlayerState<
				                                     ABlackoutPlayerState>()
			                                     : nullptr)
		{
			if (PS->SelectedClassTag.IsValid())
			{
				if (TSubclassOf<APawn> SelectedClass = CharacterRoster->
					FindPawnClassByTag(PS->SelectedClassTag))
				{
					ControllerToClass.Add(InController, SelectedClass);
					return SelectedClass.Get();
				}
			}
		}
	}

	const int32 ClassIndex = NextPlayerClassIndex % PlayerClassPool.Num();
	const TSubclassOf<APawn> SelectedPawn = PlayerClassPool[ClassIndex];
	ControllerToClass.Add(InController, SelectedPawn);
	++NextPlayerClassIndex;

	BO_LOG_NET(Log, "플레이어 #%d 클래스 분배 - %s",
	           NextPlayerClassIndex, *GetNameSafe(SelectedPawn.Get()));

	return SelectedPawn.Get();
}

void ABlackoutBattleGameMode::RegisterCutsceneManager(ABOBossIntroSequencer* InManager)
{
	CurrentCutsceneManager = InManager;
	
	if (!CurrentCutsceneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABlackoutBattleGameMode: CurrentCutsceneManager is nullPtr"))
		return;
	}
	
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		if (!bIsEncounterStarted)
		{
			bIsEncounterStarted = true;
			CurrentCutsceneManager->PlayBossIntro();
		}
	}
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
		const FString Key = MakeReconnectKey(PS->AccountId);
		if (const FGameplayTag* Saved = ReconnectStash.Find(Key))
		{
			PS->SelectedClassTag = *Saved; // 서버 Set 복제
			RespawnPlayerWithSelectedClass(NewPlayer);
			ReconnectStash.Remove(Key);
			BO_LOG_NET(Log, "재접속 복원: %s Class=%s", *Key, *Saved->ToString());
		}
		PS->ApplyBattleTransitionPolicy(EBattleTransitionType::LobbyToBattle);
	}
}

void ABlackoutBattleGameMode::HandleEmptyServerReset()
{
	ReconnectStash.Empty(); // 전원 퇴장 -> 돌아갈 사람 X
	// 매칭 서버 finish 보고 (EndMatch 내부 ) 후 데디를 로비 Idle 로 되돌려 재사용
	EndMatch(EBlackoutMatchEndReason::AllPlayersLeft);
	ReturnServerToIdleLobby();
}


void ABlackoutBattleGameMode::OnSeamlessArrival(APlayerController* PC)
{
	if (ConnectedPlayers.Num() == MaxPlayers)
	{
		StartBossCombat();
	}
}

void ABlackoutBattleGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	
	if (bIsEncounterStarted) return;
	
	if (!CurrentCutsceneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABlackoutBattleGameMode: CurrentCutsceneManager is nullPtr"))
		return;
	}
	
	int32 ReadyPlayersCount = 0;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			if (PC->GetPawn())
			{
				ReadyPlayersCount++;
			}
		}
	}

	if (ReadyPlayersCount >= MaxPlayers)
	{
		bIsEncounterStarted = true;
		if (CurrentCutsceneManager)
		{
			CurrentCutsceneManager->PlayBossIntro();
		}
	}
}

FString ABlackoutBattleGameMode::MakeReconnectKey(
	const FString& AccountId)
{
	return AccountId;
}

void ABlackoutBattleGameMode::Logout(AController* Exiting)
{
	// Super::Logout 호출 시 PlayerArray에서 PS가 제거되고 폰이 정리될 수 있으므로,
	// 이탈자를 ViewTarget으로 보고 있는 관전자들을 먼저 재지정합니다.
	if (const APlayerController* ExitingController = Cast<
		APlayerController>(Exiting))
	{
		if (ABlackoutPlayerCharacter* ExitingCharacter = Cast<
			ABlackoutPlayerCharacter>(ExitingController->GetPawn()))
		{
			RefreshSpectatorsForDeadTarget(ExitingCharacter);
		}
	}

	// 매칭 진행 중 끊김이면 재접속용으로 클래스 stash. (전원 퇴장 0명 → idle 복귀 시 일괄 clear)
	if (const ABlackoutGameState* GS = GetGameState<
		ABlackoutGameState>())
	{
		const EBlackoutMatchState MatchState = GS->CurrentMatchState;
		const bool bInMatch = MatchState !=
			EBlackoutMatchState::WaitingForPlayers && MatchState !=
			EBlackoutMatchState::Ended;
		const ABlackoutPlayerState* PS = Exiting
			                                 ? Exiting->GetPlayerState<
				                                 ABlackoutPlayerState>()
			                                 : nullptr;

		if (bInMatch && PS && PS->SelectedClassTag.IsValid())
		{
			const FString Key = MakeReconnectKey(PS->AccountId);
			if (!Key.IsEmpty())
			{
				ReconnectStash.Add(Key, PS->SelectedClassTag);
				BO_LOG_NET(Log, "재접속 stash 저장: %s Class=%s",
				           *Key, *PS->SelectedClassTag.ToString());
			}
		}
	}

	Super::Logout(Exiting);
}


void ABlackoutBattleGameMode::OnBossDefeated()
{
	BO_LOG_NET(Log, "OnBossDefeated 진입 (중복 시 델리게이트/바인딩 다중 발화 의심)");


	UBlackoutMatchFlowSubsystem* Flow = GetGameInstance()
		                                    ? GetGameInstance()->GetSubsystem<
			                                    UBlackoutMatchFlowSubsystem>()
		                                    : nullptr;

	if (!Flow)
	{
		BO_LOG_NET(Error, "OnBossDefeated: MatchFlowSubsystem 없음");
		return;
	}

	BeginBossDefeatResultFlow(Flow->GetCurrentBossType());
}

void ABlackoutBattleGameMode::BeginBossDefeatResultFlow(EBossType DefeatedBossType)
{
	if (bTravelInitiated)
	{
		BO_LOG_NET(Warning, "결과창 플로우 시작 무시: 이미 이동이 시작되었습니다.");
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(MatchResultDisplayTimerHandle) ||
		GetWorldTimerManager().IsTimerActive(MatchResultAutoTravelTimerHandle))
	{
		BO_LOG_NET(Warning, "결과창 플로우 시작 무시: 이미 결과창 타이머가 진행 중입니다.");
		return;
	}

	PendingResultBossType = DefeatedBossType;
	GetWorldTimerManager().ClearTimer(MatchResultDisplayTimerHandle);
	GetWorldTimerManager().ClearTimer(MatchResultAutoTravelTimerHandle);

	BO_LOG_NET(Log,
	           "보스 처치 결과창 예약: Boss=%s DisplayDelay=%.2f AutoTravelDelay=%.2f",
	           *UEnum::GetValueAsString(PendingResultBossType),
	           MatchResultDisplayDelay,
	           MatchResultAutoTravelDelay);

	if (MatchResultDisplayDelay <= KINDA_SMALL_NUMBER)
	{
		ShowMatchResultAfterDelay();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MatchResultDisplayTimerHandle,
		this,
		&ABlackoutBattleGameMode::ShowMatchResultAfterDelay,
		MatchResultDisplayDelay,
		false);
}

void ABlackoutBattleGameMode::ShowMatchResultAfterDelay()
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		BO_LOG_NET(Error, "결과창 표시 실패: GameState가 비어 있습니다.");
		return;
	}

	TArray<ABlackoutPlayerState*> Participants;
	SnapshotMatchResultParticipants(Participants);

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float AutoTravelTime = CurrentTime + FMath::Max(0.f, MatchResultAutoTravelDelay);
	GS->SetMatchResultState(
		true,
		PendingResultBossType,
		CurrentTime,
		AutoTravelTime,
		Participants);

	if (MatchResultAutoTravelDelay <= KINDA_SMALL_NUMBER)
	{
		AutoTravelAfterMatchResult();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MatchResultAutoTravelTimerHandle,
		this,
		&ABlackoutBattleGameMode::AutoTravelAfterMatchResult,
		MatchResultAutoTravelDelay,
		false);
}

void ABlackoutBattleGameMode::AutoTravelAfterMatchResult()
{
	BO_LOG_NET(Log, "결과창 자동 이동 타이머 만료");
	ExecuteMatchResultTravel();
}

void ABlackoutBattleGameMode::ExecuteMatchResultTravel()
{
	GetWorldTimerManager().ClearTimer(MatchResultDisplayTimerHandle);
	GetWorldTimerManager().ClearTimer(MatchResultAutoTravelTimerHandle);

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		TArray<ABlackoutPlayerState*> EmptyParticipants;
		GS->SetMatchResultState(false, PendingResultBossType, 0.f, 0.f, EmptyParticipants);
	}

	if (PendingResultBossType == EBossType::Mid)
	{
		if (UBlackoutMatchFlowSubsystem* Flow = GetGameInstance()
			? GetGameInstance()->GetSubsystem<UBlackoutMatchFlowSubsystem>()
			: nullptr)
		{
			Flow->AdvanceStage();
		}
		else
		{
			BO_LOG_NET(Error, "중간 보스 결과 이동 실패: MatchFlowSubsystem 없음");
			return;
		}

		BO_LOG_NET(Log, "중간 보스 결과창 종료 — 로비로 이동");
		TravelToLobby(FLinearColor::White);
		return;
	}

	BO_LOG_NET(Log, "메인 보스 결과창 종료 — 타이틀로 이동");
	EndMatch(EBlackoutMatchEndReason::BossDefeated);
	TravelToTitle();
}

void ABlackoutBattleGameMode::SnapshotMatchResultParticipants(
	TArray<ABlackoutPlayerState*>& OutParticipants) const
{
	OutParticipants.Reset();

	const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		return;
	}

	OutParticipants.Reserve(GS->PlayerArray.Num());
	for (APlayerState* PlayerStateBase : GS->PlayerArray)
	{
		if (ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase))
		{
			OutParticipants.Add(BlackoutPlayerState);
		}
	}
}

void ABlackoutBattleGameMode::RegisterBoss(ABlackoutBossCharacter* Boss)
{
	if (!Boss)
	{
		return;
	}
	// 보스 BeginPlay OnDefeated 바인딩
	Boss->OnDefeated.AddUObject(this, &ABlackoutBattleGameMode::OnBossDefeated);
	BO_LOG_NET(Log, "보스 등록: %s — OnDefeated 바인딩", *GetNameSafe(Boss))
}

void ABlackoutBattleGameMode::BO_SimBossDefeated()
{
	BO_LOG_NET(Warning, "[테스트] BO_SimMidBossDefeated — 중간 보스 처치 시뮬레이션");
	OnBossDefeated();
}

void ABlackoutBattleGameMode::BO_SimPartyWipe()
{
	BO_LOG_NET(Warning, "[테스트] BO_SimPartyWipe — 파티 전멸 시뮬레이션");
	HandlePartyWipe();
}

void ABlackoutBattleGameMode::InitGame(const FString& MapName,
                                       const FString& Options,
                                       FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// 위치정보 시작 
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->BeginRun();
	}

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

void ABlackoutBattleGameMode::RespawnPlayerWithSelectedClass(
	APlayerController* InController)
{
	// 캐시 무효화
	if (InController)
	{
		ControllerToClass.Remove(InController);
	}

	Super::RespawnPlayerWithSelectedClass(InController);
}

void ABlackoutBattleGameMode::InitGameState()
{
	Super::InitGameState();

	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::WaitingForPlayers);
		// 초기 상태(전이표 비대상)
	}
}

void ABlackoutBattleGameMode::PreLogin(const FString& Options,
                                       const FString& Address,
                                       const FUniqueNetIdRepl& UniqueId,
                                       FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return; // 상위에서 이미 거부
	}
	// stash 에 있는 Account = 재접속 -> 진행중이여도 허용
	const FString Key = MakeReconnectKey(
		UGameplayStatics::ParseOption(Options, TEXT("Acc")));
	if (!Key.IsEmpty() &&
		ReconnectStash.Contains(Key))
	{
		BO_LOG_NET(Log, "PreLogin 재접속 허용: %s", *Key);
		return;
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

void ABlackoutBattleGameMode::TravelToLobby(FLinearColor FadeColor)
{
	if (bTravelInitiated)
	{
		BO_LOG_NET(Warning, "TravelToLobby 중복 호출 무시 — ServerTravel 진행 중");
		return;
	}
	if (!LobbyMapPath.IsValid())
	{
		BO_LOG_NET(
			Error,
			"TravelToLobby 실패: LobbyMapPath 미설정 (BP_BlackoutBattleGameMode 확인)");
		return;
	}
	bTravelInitiated = true;
	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		GS->SetMatchState(EBlackoutMatchState::Starting);
	}
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->FlushPending();
	}
	BroadcastScreenFadeOut(FadeColor);
	GetWorldTimerManager().SetTimer(FadeTravelTimerHandle, this,
	                                &ABlackoutBattleGameMode::DoTravelToLobby,
	                                FadeOutTravelDelay, false);
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
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->EndRun();
	}
}

void ABlackoutBattleGameMode::NotifyPlayerFullyDead(
	ABlackoutPlayerCharacter* DeadPlayer)
{
	if (!DeadPlayer)
	{
		BO_LOG_NET(Warning, "완전 사망 알림 무시: DeadPlayer가 비어 있음");
		return;
	}
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->AddEvent(DeadPlayer , TEXT("death"));
	}

	BO_LOG_NET(Log, "플레이어 완전 사망 감지: %s", *GetNameSafe(DeadPlayer));
	RefreshSpectatorsForDeadTarget(DeadPlayer);

	if (ABlackoutPlayerController* DeadPlayerController = Cast<
		ABlackoutPlayerController>(DeadPlayer->GetController()))
	{
		AssignSpectateTargetForDeadPlayer(DeadPlayerController);
	}
	else
	{
		BO_LOG_NET(Warning, "관전 전환 실패: 사망 플레이어 컨트롤러를 찾을 수 없음 Player=%s",
		           *GetNameSafe(DeadPlayer));
	}

	EvaluatePartyWipe();
}

void ABlackoutBattleGameMode::NotifyPlayerDowned(
	ABlackoutPlayerCharacter* DownedPlayer)
{
	if (!DownedPlayer)
	{
		return;
	}
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->AddEvent(DownedPlayer , TEXT("down"));
	}
}

void ABlackoutBattleGameMode::EvaluatePartyWipe()
{
	ABlackoutGameState* BlackoutGameState = GetGameState<ABlackoutGameState>();
	if (!BlackoutGameState)
	{
		BO_LOG_NET(Error, "전멸 평가 실패: GameState가 비어 있음");
		return;
	}

	// 비전투 페이즈(쉘터/대기/종료)에서는 평가하지 않음.
	// 레거시 InCombat 값은 의도적으로 미포함 — 새 런-페이즈에서는 진입하지 않음.
	const EBlackoutMatchState MS = BlackoutGameState->CurrentMatchState;
	const bool bSkipEvaluation =
		MS == EBlackoutMatchState::WaitingForPlayers ||
		MS == EBlackoutMatchState::ShelterPrep ||
		MS == EBlackoutMatchState::Ended;

	if (bSkipEvaluation)
	{
		BO_LOG_NET(Verbose, "전멸 평가 스킵: 비전투 페이즈 State=%d",
		           static_cast<int32>(MS));
		return;
	}

	int32 AlivePlayers = 0;
	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(
			PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		APlayerController* PlayerController = BlackoutPlayerState->
			GetPlayerController();
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
		                                                ? Cast<
			                                                ABlackoutPlayerCharacter>(
			                                                SpectatorController
			                                                ->GetPawn())
		                                                : nullptr;

	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPlayerState = Cast<
			ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		APlayerController* CandidateController = BlackoutPlayerState->
			GetPlayerController();
		ABlackoutPlayerCharacter* Candidate = CandidateController
			                                      ? Cast<
				                                      ABlackoutPlayerCharacter>(
				                                      CandidateController->
				                                      GetPawn())
			                                      : nullptr;

		if (!Candidate || Candidate == SpectatorPawn || Candidate->IsDead())
		{
			continue;
		}

		return Candidate;
	}

	return nullptr;
}

void ABlackoutBattleGameMode::AssignSpectateTargetForDeadPlayer(
	ABlackoutPlayerController* SpectatorController)
{
	if (!SpectatorController)
	{
		BO_LOG_NET(Warning, "관전 대상 지정 실패: SpectatorController가 비어 있음");
		return;
	}

	ABlackoutPlayerCharacter* SpectateTarget = FindInitialSpectateTarget(
		SpectatorController);
	if (!SpectateTarget)
	{
		// 살아있는 아군이 없으면 자기 시신을 ViewTarget으로 두어 0,0,0 카메라 점프를 막습니다.
		if (APawn* OwnPawn = SpectatorController->GetPawn())
		{
			SpectatorController->EnterSpectatorMode();
			SpectatorController->SetViewTargetWithBlend(OwnPawn, 0.35f);
			SpectatorController->Client_SetSpectateTarget(OwnPawn, 0.35f);
			BO_LOG_NET(Log, "관전 대상 폴백(자기 시신): Controller=%s",
			           *GetNameSafe(SpectatorController));
		}
		else
		{
			BO_LOG_NET(Warning, "관전 대상 폴백 실패: Pawn이 비어 있음 Controller=%s",
			           *GetNameSafe(SpectatorController));
		}
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

void ABlackoutBattleGameMode::CycleSpectateTargetForSpectator(
	ABlackoutPlayerController* SpectatorController, int32 Direction)
{
	if (!SpectatorController || !GameState)
	{
		return;
	}

	const ABlackoutPlayerCharacter* SpectatorPawn = Cast<
		ABlackoutPlayerCharacter>(SpectatorController->GetPawn());
	if (!SpectatorPawn || !SpectatorPawn->IsDead())
	{
		BO_LOG_NET(Verbose, "관전 대상 순환 무시: 사망 상태가 아닙니다. Controller=%s",
		           *GetNameSafe(SpectatorController));
		return;
	}

	// 후보 = 살아있는 다른 플레이어 캐릭터. 다운 상태도 후보에 포함합니다(설계 기준 살아있는 아군).
	TArray<ABlackoutPlayerCharacter*> Candidates;
	Candidates.Reserve(GameState->PlayerArray.Num());
	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPlayerState = Cast<
			ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		APlayerController* CandidateController = BlackoutPlayerState->
			GetPlayerController();
		ABlackoutPlayerCharacter* Candidate = CandidateController
			                                      ? Cast<
				                                      ABlackoutPlayerCharacter>(
				                                      CandidateController->
				                                      GetPawn())
			                                      : nullptr;
		if (!Candidate || Candidate == SpectatorPawn || Candidate->IsDead())
		{
			continue;
		}

		Candidates.Add(Candidate);
	}

	if (Candidates.Num() == 0)
	{
		// 후보가 없으면 자기 시신으로 폴백해 카메라가 월드 원점으로 튀지 않게 합니다.
		if (APawn* OwnPawn = SpectatorController->GetPawn())
		{
			SpectatorController->EnterSpectatorMode();
			SpectatorController->SetViewTargetWithBlend(OwnPawn, 0.25f);
			SpectatorController->Client_SetSpectateTarget(OwnPawn, 0.25f);
		}
		BO_LOG_NET(Log, "관전 대상 순환 실패(폴백 적용): 살아있는 아군이 없음 Controller=%s",
		           *GetNameSafe(SpectatorController));
		return;
	}

	const AActor* CurrentViewTarget = SpectatorController->GetViewTarget();
	const int32 CurrentIndex = Candidates.IndexOfByPredicate(
		[CurrentViewTarget](const ABlackoutPlayerCharacter* C)
		{
			return C == CurrentViewTarget;
		});

	const int32 Step = (Direction >= 0) ? 1 : -1;
	const int32 NextIndex = (CurrentIndex == INDEX_NONE)
		                        ? 0
		                        : ((CurrentIndex + Step + Candidates.Num()) %
			                        Candidates.Num());

	ABlackoutPlayerCharacter* NextTarget = Candidates[NextIndex];
	SpectatorController->EnterSpectatorMode();
	SpectatorController->SetViewTargetWithBlend(NextTarget, 0.25f);
	SpectatorController->Client_SetSpectateTarget(NextTarget, 0.25f);

	BO_LOG_NET(Log,
	           "관전 대상 순환: Spectator=%s Direction=%d Target=%s",
	           *GetNameSafe(SpectatorController),
	           Direction,
	           *GetNameSafe(NextTarget));
}

void ABlackoutBattleGameMode::RefreshSpectatorsForDeadTarget(
	ABlackoutPlayerCharacter* DeadTarget)
{
	if (!DeadTarget || !GameState)
	{
		return;
	}

	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPlayerState = Cast<
			ABlackoutPlayerState>(PlayerStateBase);
		if (!BlackoutPlayerState)
		{
			continue;
		}

		ABlackoutPlayerController* SpectatorController = Cast<
			ABlackoutPlayerController>(
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

void ABlackoutBattleGameMode::DoTravelToLobby()
{
	const FString PackageName = LobbyMapPath.GetLongPackageName();
	BO_LOG_NET(Log, "TravelToLobby — ServerTravel -> %s", *PackageName);
	GetWorld()->ServerTravel(PackageName);
}

void ABlackoutBattleGameMode::DoTravelToTitle()
{
	const FString URL = TitleMapPath.GetLongPackageName();
	BO_LOG_NET(Log, "메인보스 클리어 — 전 클라 타이틀 ClientTravel -> %s", *URL);
	for (const TObjectPtr<APlayerController>& PC : ConnectedPlayers)
	{
		if (PC)
		{
			PC->ClientTravel(URL, TRAVEL_Absolute);
		}
	}

	// 다음 매치를 위해 매치 진행 인덱스 초기화 + 서버 idle 로비 복귀
	ReturnServerToIdleLobby();
}

void ABlackoutBattleGameMode::ReturnServerToIdleLobby()
{
	if (UBlackoutTelemetrySampler* Sampler = GetTelemetrySampler())
	{
		Sampler->EndRun();
	}
	
	if (UBlackoutMatchFlowSubsystem* FlowSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UBlackoutMatchFlowSubsystem>()
		: nullptr)
	{
		FlowSubsystem->ResetStages();
	}
	if (LobbyMapPath.IsValid())
	{
		const FString LobbyPackage = LobbyMapPath.GetLongPackageName();
		BO_LOG_NET(Log, "서버 idle 복귀 — ServerTravel -> %s", *LobbyPackage);
		GetWorld()->ServerTravel(LobbyPackage);
	}
}

UBlackoutTelemetrySampler* ABlackoutBattleGameMode::
GetTelemetrySampler() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UBlackoutTelemetrySampler>() : nullptr;
}

// 파티 전멸 감지 시 호출. 체크포인트 텔레포트 + PartyWipeRestart 정책 + Ready 리셋 + InCombatReady 복귀.
void ABlackoutBattleGameMode::HandlePartyWipe()
{
	Super::HandlePartyWipe();

	TravelToLobby(FLinearColor::Black);
}

void ABlackoutBattleGameMode::StartSurrenderVote(
	ABlackoutPlayerController* Proposer)
{
	if (!Proposer || !GameState)
	{
		return;
	}

	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		return;
	}

	// 1. 쿨다운 검사
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (CurrentTime < GS->SurrenderVoteCooldownEndTime)
	{
		BO_LOG_NET(Warning, "항복 투표 발의 거부: 쿨다운 중입니다. (남은 시간: %.1fs)",
		           GS->SurrenderVoteCooldownEndTime - CurrentTime);
		return;
	}

	// 2. 매치 상태 검사 (전투 중이어야 함)
	if (!IsActiveCombatState(GS->CurrentMatchState))
	{
		BO_LOG_NET(Warning, "항복 투표 발의 거부: 전투 중이 아닙니다. (State=%s)",
		           *UEnum::GetValueAsString(GS->CurrentMatchState));
		return;
	}

	// 3. 발의자 자격 검사 (쓰러졌거나 완전히 사망한 플레이어만 가능)
	const ABlackoutPlayerCharacter* ProposerPawn = Cast<
		ABlackoutPlayerCharacter>(Proposer->GetPawn());
	const bool bCanPropose = ProposerPawn && (ProposerPawn->IsDowned() ||
		ProposerPawn->IsDead());
	if (!bCanPropose)
	{
		BO_LOG_NET(Warning, "항복 투표 발의 거부: 생존자는 항복을 발의할 수 없습니다. (Proposer=%s)",
		           *Proposer->GetName());
		return;
	}

	// 4. 솔로 플레이어 예외 차단
	if (GameState->PlayerArray.Num() <= 1)
	{
		BO_LOG_NET(Warning, "항복 투표 발의 거부: 1인 솔로 플레이 시에는 투표를 할 수 없습니다.");
		return;
	}

	// 5. 이미 활성화된 투표가 있는 경우 무시
	if (GS->bIsSurrenderVoteActive)
	{
		return;
	}

	BO_LOG_NET(Log, "항복 투표 발의 성공: 발의자=%s", *Proposer->GetName());

	// 6. 상태 변수 초기화
	GS->bIsSurrenderVoteActive = true;
	GS->SurrenderVoteYesCount = 0;
	GS->SurrenderVoteNoCount = 0;
	GS->SurrenderVoteEndTimeSeconds = CurrentTime + 30.f;

	for (APlayerState* PSBase : GameState->PlayerArray)
	{
		if (ABlackoutPlayerState* PS = Cast<ABlackoutPlayerState>(PSBase))
		{
			PS->bRequestedSurrender = false;
			PS->bVotedAgainstSurrender = false;
		}
	}

	// 7. 발의자 자동 찬성
	if (ABlackoutPlayerState* ProposerPS = Proposer->GetPlayerState<
		ABlackoutPlayerState>())
	{
		ProposerPS->bRequestedSurrender = true;
	}

	// 8. 30초 타이머 예약
	GetWorldTimerManager().ClearTimer(SurrenderVoteTimerHandle);
	GetWorldTimerManager().SetTimer(
		SurrenderVoteTimerHandle,
		this,
		&ABlackoutBattleGameMode::TimeoutSurrenderVote,
		30.f,
		false
	);

	// 9. 모든 플레이어에게 투표 IMC 푸시
	SetAllPlayersSurrenderInputContextActive(true);

	// 10. 투표 평가 실행
	EvaluateSurrenderVote();
}

void ABlackoutBattleGameMode::CastSurrenderVote(
	ABlackoutPlayerController* Voter, bool bAgree)
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS || !GS->bIsSurrenderVoteActive)
	{
		return;
	}

	ABlackoutPlayerState* VoterPS = Voter
		                                ? Voter->GetPlayerState<
			                                ABlackoutPlayerState>()
		                                : nullptr;
	if (!VoterPS)
	{
		return;
	}

	VoterPS->bRequestedSurrender = bAgree;
	VoterPS->bVotedAgainstSurrender = !bAgree;

	BO_LOG_NET(Log, "투표 접수: %s -> %s", *Voter->GetName(),
	           bAgree ? TEXT("찬성") : TEXT("반대"));

	EvaluateSurrenderVote();
}

void ABlackoutBattleGameMode::EvaluateSurrenderVote()
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS || !GS->bIsSurrenderVoteActive)
	{
		return;
	}

	const int32 TotalPlayerCount = GameState->PlayerArray.Num();
	const int32 Required = FMath::Max(1, (TotalPlayerCount / 2) + 1);

	int32 YesCount = 0;
	int32 NoCount = 0;

	for (APlayerState* PSBase : GameState->PlayerArray)
	{
		if (const ABlackoutPlayerState* PS = Cast<ABlackoutPlayerState>(PSBase))
		{
			if (PS->bRequestedSurrender)
			{
				++YesCount;
			}
			else if (PS->bVotedAgainstSurrender)
			{
				++NoCount;
			}
		}
	}

	GS->SurrenderVoteYesCount = YesCount;
	GS->SurrenderVoteNoCount = NoCount;
	GS->RequiredSurrenderVoteCount = Required;

	BO_LOG_NET(Log, "항복 투표 평가: 찬성=%d, 반대=%d, 필요=%d, 접속인원=%d", YesCount, NoCount,
	           Required, TotalPlayerCount);

	// 가결 성공 조건
	if (YesCount >= Required)
	{
		HandleSurrenderSuccess();
		return;
	}

	// 조기 기각 조건 (반대표가 너무 많아 남은 미투표자가 전원 찬성해도 도달이 안 될 때)
	if ((TotalPlayerCount - NoCount) < Required)
	{
		HandleSurrenderFailed(false);
		return;
	}

	// 수치 변경을 델리게이트 브로드캐스트하여 동기화
	GS->OnSurrenderVoteStateChanged.Broadcast(
		GS->bIsSurrenderVoteActive,
		YesCount,
		NoCount,
		GS->SurrenderVoteEndTimeSeconds
	);
}

void ABlackoutBattleGameMode::TimeoutSurrenderVote()
{
	BO_LOG_NET(Log, "항복 투표 기각: 제한 시간(30초) 초과");
	HandleSurrenderFailed(true);
}

void ABlackoutBattleGameMode::StartBossCombat()
{
	BO_LOG_NET(Log, "StartBossCombat 진입 (StartBossCombat 진입 — 전투 상태 전이)");

	const UBlackoutMatchFlowSubsystem* Flow = GetGameInstance()
		                                          ? GetGameInstance()->
		                                          GetSubsystem<
			                                          UBlackoutMatchFlowSubsystem>()
		                                          : nullptr;

	if (!Flow)
	{
		BO_LOG_NET(Error, "StartBossCombat: MatchFlowSubsystem 없음");
		return;
	}

	const EBlackoutMatchState NewState = (Flow->GetCurrentBossType() ==
		                                     EBossType::Mid)
		                                     ? EBlackoutMatchState::MidBossCombat
		                                     : EBlackoutMatchState::MainBossCombat;

	TransitionTo(NewState);
	BO_LOG_NET(Log, "보스맵 전원 도착 — 전투 시작 (%s)",
	           *UEnum::GetValueAsString(NewState));
}

void ABlackoutBattleGameMode::TravelToTitle()
{
	if (!TitleMapPath.IsValid())
	{
		BO_LOG_NET(
			Error,
			"TravelToTitle 실패: TitleMapPath 미설정 (BP_BlackoutBattleGameMode 확인)");
		return;
	}

	BroadcastScreenFadeOut(FLinearColor::White);
	GetWorldTimerManager().SetTimer(FadeTravelTimerHandle, this,
	                                &ABlackoutBattleGameMode::DoTravelToTitle,
	                                FadeOutTravelDelay, false);
}

void ABlackoutBattleGameMode::HandleSurrenderSuccess()
{
	BO_LOG_NET(Log, "항복 투표 가결 완료! 체크포인트로 퇴각합니다.");

	GetWorldTimerManager().ClearTimer(SurrenderVoteTimerHandle);
	SetAllPlayersSurrenderInputContextActive(false);

	// 복귀 수행
	HandlePartyWipe();

	// 투표 데이터 정리
	ClearSurrenderVotes();
}

void ABlackoutBattleGameMode::HandleSurrenderFailed(bool bIsTimeout)
{
	BO_LOG_NET(Log, "항복 투표 부결 및 기각 처리.");

	GetWorldTimerManager().ClearTimer(SurrenderVoteTimerHandle);
	SetAllPlayersSurrenderInputContextActive(false);

	// 1분(60초) 발의 제한 쿨다운 설정
	if (ABlackoutGameState* GS = GetGameState<ABlackoutGameState>())
	{
		const float CurrentTime = GetWorld()
			                          ? GetWorld()->GetTimeSeconds()
			                          : 0.f;
		GS->SurrenderVoteCooldownEndTime = CurrentTime + 60.f;
	}

	ClearSurrenderVotes();
}

void ABlackoutBattleGameMode::ClearSurrenderVotes()
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		return;
	}

	GS->bIsSurrenderVoteActive = false;
	GS->SurrenderVoteYesCount = 0;
	GS->SurrenderVoteNoCount = 0;
	GS->SurrenderVoteEndTimeSeconds = 0.f;

	for (APlayerState* PSBase : GameState->PlayerArray)
	{
		if (ABlackoutPlayerState* PS = Cast<ABlackoutPlayerState>(PSBase))
		{
			PS->bRequestedSurrender = false;
			PS->bVotedAgainstSurrender = false;
		}
	}

	GS->OnSurrenderVoteStateChanged.Broadcast(false, 0, 0, 0.f);
}

void ABlackoutBattleGameMode::SetAllPlayersSurrenderInputContextActive(
	bool bActive)
{
	if (!GameState)
	{
		return;
	}

	for (APlayerState* PSBase : GameState->PlayerArray)
	{
		if (const ABlackoutPlayerState* PS = Cast<ABlackoutPlayerState>(PSBase))
		{
			if (ABlackoutPlayerController* PC = Cast<ABlackoutPlayerController>(
				PS->GetPlayerController()))
			{
				PC->Client_SetSurrenderInputContextActive(bActive);
			}
		}
	}
}
