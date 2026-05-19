#include "BlackoutGameMode.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutLog.h"
#include "Core/BlackoutTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

namespace
{
	// 단일맵 런-페이즈 허용 전이표. 그 외 전이는 거부.
	bool IsAllowedMatchTransition(EBlackoutMatchState From, EBlackoutMatchState To)
	{
		switch (From)
		{
		case EBlackoutMatchState::WaitingForPlayers:
			return To == EBlackoutMatchState::ShelterPrep;
		case EBlackoutMatchState::ShelterPrep:
			return To == EBlackoutMatchState::MidBossCombat;
		case EBlackoutMatchState::MidBossCombat:
			return To == EBlackoutMatchState::ShelterMid       // Shrewd 처치
				|| To == EBlackoutMatchState::ShelterPrep      // 전멸 회귀
				|| To == EBlackoutMatchState::Ended;           // 이탈/타임아웃
		case EBlackoutMatchState::ShelterMid:
			return To == EBlackoutMatchState::MainBossCombat;
		case EBlackoutMatchState::MainBossCombat:
			return To == EBlackoutMatchState::ShelterMid       // 전멸 회귀
				|| To == EBlackoutMatchState::Ended;           // 승리/이탈/타임아웃
		default:
			return false;
		}
	}
}

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

// Ready 집계 기본 구현. 정원 미달이거나 한 명이라도 bIsReady == false 면 false.
bool ABlackoutGameMode::AllPlayersReady() const
{
	if (ConnectedPlayers.Num() < MaxPlayers) { return false; }
	if (!GameState) { return false; }

	for (APlayerState* PS : GameState->PlayerArray)
	{
		const ABlackoutPlayerState* BlackoutPS = Cast<ABlackoutPlayerState>(PS);
		if (!BlackoutPS || !BlackoutPS->bIsReady) { return false; }
	}
	return true;
}

// Ready 상태 갱신 직후 호출. 성립 시 자식 GameMode 훅 실행.
void ABlackoutGameMode::NotifyReadyChanged()
{
	if (AllPlayersReady())
	{
		OnAllPlayersReady();
	}
}

// 매치 상태 전이 단일 권위. 흩어진 SetMatchState 직접호출을 Step3 에서 이리로 수렴.
void ABlackoutGameMode::TransitionTo(EBlackoutMatchState NewState)
{
	ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	if (!GS)
	{
		return;
	}

	const EBlackoutMatchState Current = GS->CurrentMatchState;
	if (Current == NewState)
	{
		return;
	}
	if (!IsAllowedMatchTransition(Current, NewState))
	{
		BO_LOG_NET(Warning, "거부된 매치 전이: %s -> %s",
			*UEnum::GetValueAsString(Current), *UEnum::GetValueAsString(NewState));
		return;
	}

	GS->SetMatchState(NewState);  // 복제 + 전이 로그는 SetMatchState 가 처리
}
