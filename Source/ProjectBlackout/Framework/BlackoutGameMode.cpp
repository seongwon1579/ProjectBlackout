#include "BlackoutGameMode.h"

#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutGameState.h"
#include "BlackoutPlayerState.h"
#include "BlackoutPlayerController.h"
#include "BlackoutLog.h"
#include "BOCharacterRoster.h"
#include "Core/BlackoutTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

namespace
{
	// 단일맵 런-페이즈 허용 전이표. 그 외 전이는 거부.
	bool IsAllowedMatchTransition(EBlackoutMatchState From,
	                              EBlackoutMatchState To)
	{
		switch (From)
		{
		// TransitionTo 호출처는 로비(HandleLobbyArrival)와 보스맵(StartBossCombat)뿐 —
		// 둘 다 WaitingForPlayers 출발. 클리어/와이프/타이틀은 SetMatchState raw 라 전이표 비대상.
		case EBlackoutMatchState::WaitingForPlayers:
			return To == EBlackoutMatchState::ShelterPrep    // 로비 ShelterPrep
				|| To == EBlackoutMatchState::MidBossCombat   // 보스맵 seamless 도착
				|| To == EBlackoutMatchState::MainBossCombat; // 보스맵 seamless 도착
		default:
			return false;
		}
	}
}

ABlackoutGameMode::ABlackoutGameMode()
{
	GameStateClass = ABlackoutGameState::StaticClass();
	PlayerStateClass = ABlackoutPlayerState::StaticClass();
	PlayerControllerClass = ABlackoutPlayerController::StaticClass();

	// Seamless Travel: 맵 이동 시 PlayerController/PlayerState 유지 (커넥션 끊김 없이 상태 이관).
	bUseSeamlessTravel = true;
}

void ABlackoutGameMode::InitGame(const FString& MapName, const FString& Options,
                                 FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 매칭 API(Nest.js)가 데디 실행 시 ?SessionId=<UUID> 형태로 넘겨주는 식별자를 보관한다.
	MatchmakingSessionId = UGameplayStatics::ParseOption(
		Options, TEXT("SessionId"));
	BO_LOG_NET(Log, "InitGame: Map=%s SessionId=%s", *MapName,
	           *MatchmakingSessionId);
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
	BO_LOG_NET(Log, "Player logged out: %s (remaining=%d)",
	           *GetNameSafe(Exiting), ConnectedPlayers.Num());

	OnPlayerLeft(Exiting);
}

void ABlackoutGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	if (APlayerController* PC =Cast<APlayerController>(C))
	{
		ConnectedPlayers.AddUnique(PC);
		OnSeamlessArrival(PC);
	}
}

void ABlackoutGameMode::HandlePartyWipe()
{
	BO_LOG_CORE(Log, "HandlePartyWipe triggered");
}

void ABlackoutGameMode::RespawnPlayerWithSelectedClass(
	APlayerController* InController)
{
	if (!InController)
	{
		return;
	}

	const ABlackoutGameState* GS = GetGameState<ABlackoutGameState>();
	const UBOCharacterRoster* CharacterRoster = GS
		                                            ? GS->CharacterRoster
		                                            : nullptr;

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
	BO_LOG_NET(Log, "캐릭터 교체: %s -> %s",
	           *InController->GetName(), *GetNameSafe(NewClass.Get()));
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

// 매치 상태 전이 단일 권위. SetMatchState 직접 호출을 이 진입점으로 일원화한다.
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
		           *UEnum::GetValueAsString(Current),
		           *UEnum::GetValueAsString(NewState));
		return;
	}

	GS->SetMatchState(NewState); // 복제 + 전이 로그는 SetMatchState 가 처리

	// 쉘터 진입 시 Ready 를 새로 시작한다 (비석 상호작용으로 다시 커밋).
	if (NewState == EBlackoutMatchState::ShelterPrep)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (ABlackoutPlayerState* BPS = Cast<ABlackoutPlayerState>(PS))
			{
				BPS->bIsReady = false;
			}
		}
	}
}

void ABlackoutGameMode::BroadcastScreenFadeOut(FLinearColor FadeColor)
{
	
	for (const TObjectPtr<APlayerController>& PC : ConnectedPlayers)
	{
		if (ABlackoutPlayerController* BlackoutPlayerController = Cast<ABlackoutPlayerController>(PC))
		{
			BlackoutPlayerController->Client_StartScreenFadeOut(FadeColor);
		}
	}
}
