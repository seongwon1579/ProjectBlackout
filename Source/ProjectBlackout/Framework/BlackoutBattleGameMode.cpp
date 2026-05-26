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
	if (const UBOCharacterRoster* CharacterRoster = GS? GS->CharacterRoster : nullptr )
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
					if (ABlackoutPlayerController* BPC = Cast<
						ABlackoutPlayerController>(PC))
					{
						if (BPC->IsLocalController())
						{
							// Listen Server host PC — Client RPC 가 OwningConnection nullptr 로 skip 되므로 직접 호출
							BPC->Client_OpenClassSelectUI_Implementation();
						}
						else
						{
							BPC->Client_OpenClassSelectUI();
						}
					}
				}

				if (bAutoStartOnFull)
				{
					BO_LOG_NET(Warning,
					           "[테스트] bAutoStartOnFull — 클래스선택/Ready 생략. 출시 빌드면 비활성 필요");
					for (APlayerState* PS : GameState->PlayerArray)
					{
						if (ABlackoutPlayerState* BPS = Cast<
							ABlackoutPlayerState>(PS))
						{
							BPS->bIsReady = true;
						}
					}
					NotifyReadyChanged(); // 정상 Ready 경로로 합류
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

	Super::Logout(Exiting);
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
		return; // 쉘터 페이즈가 아니면 무시
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
	if (!InController)
	{
		return;
	}
	
	const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	const UBOCharacterRoster* CharacterRoster = GS ? GS->CharacterRoster : nullptr;
	
	if (!CharacterRoster)
	{
		return;
	}

	ABlackoutPlayerState* PS = InController->GetPlayerState<
		ABlackoutPlayerState>();
	if (!PS || !PS->SelectedClassTag.IsValid())
	{
		return;
	}

	TSubclassOf<APawn> NewClass = CharacterRoster->FindPawnClassByTag(
		PS->SelectedClassTag);
	if (!NewClass)
	{
		return;
	}
	FTransform SpawnTransform = FTransform::Identity;
	if (APawn* OldPawn = InController->GetPawn())
	{
		SpawnTransform = OldPawn->GetActorTransform();
		if (UBlackoutAbilitySystemComponent* BlackoutASC = PS->
			GetBlackoutAbilitySystemComponent())
		{
			BlackoutASC->ClearAllAbilities();
		}
		InController->UnPossess();
		OldPawn->Destroy();
	}

	ControllerToClass.Remove(InController);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InController;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
		NewClass, SpawnTransform, SpawnParams);
	if (!NewPawn)
	{
		return;
	}

	InController->Possess(NewPawn);
	ControllerToClass.Add(InController, NewClass);
	BO_LOG_NET(Log, "캐릭터 교체: %s -> %s",
	           *InController->GetName(), *GetNameSafe(NewClass.Get()));
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

void ABlackoutBattleGameMode::NotifyPlayerFullyDead(
	ABlackoutPlayerCharacter* DeadPlayer)
{
	if (!DeadPlayer)
	{
		BO_LOG_NET(Warning, "완전 사망 알림 무시: DeadPlayer가 비어 있음");
		return;
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
		MS == EBlackoutMatchState::ShelterMid ||
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

	// 체크포인트 주위 방사형 분산. 단일 좌표 텔레포트 시 캡슐 콜리전이 인터록되어 움직임이 막히는 문제 회피.
	constexpr float RespawnRadius = 150.f;
	const int32 NumPlayers = GameState->PlayerArray.Num();

	for (int32 Index = 0; Index < NumPlayers; ++Index)
	{
		ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(
			GameState->PlayerArray[Index]);
		if (!BlackoutPS)
		{
			continue;
		}

		BlackoutPS->bIsReady = false;
		BlackoutPS->ApplyBattleTransitionPolicy(
			EBattleTransitionType::PartyWipeRestart);

		if (APlayerController* PC = BlackoutPS->GetPlayerController())
		{
			if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<
				ABlackoutPlayerCharacter>(PC->GetPawn()))
			{
				PlayerCharacter->RestoreFromPartyWipeRestart();
			}

			if (ABlackoutPlayerController* BlackoutPC = Cast<
				ABlackoutPlayerController>(PC))
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
				const float Angle = (2.f * PI) * static_cast<float>(Index) /
					FMath::Max(NumPlayers, 1);
				const FVector Offset(FMath::Cos(Angle) * RespawnRadius,
				                     FMath::Sin(Angle) * RespawnRadius, 0.f);
				Pawn->SetActorLocation(RespawnLocation + Offset, false, nullptr,
				                       ETeleportType::TeleportPhysics);
			}
		}
	}

	if (CurrentArena)
	{
		IBlackoutArenaResettableInterface::Execute_ResetArena(
			CurrentArena.GetObject());
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
