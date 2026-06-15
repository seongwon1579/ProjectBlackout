#include "UI/BlackoutMatchResultWidgetController.h"

#include "Core/BlackoutLog.h"
#include "Framework/BlackoutGameState.h"
#include "Framework/BlackoutPlayerState.h"
#include "GameFramework/PlayerController.h"

void UBlackoutMatchResultWidgetController::BeginDestroy()
{
	UnbindAllPlayers();

	if (ABlackoutGameState* BlackoutGameState = GameState.Get())
	{
		BlackoutGameState->OnPlayerArrayChanged.RemoveAll(this);
		BlackoutGameState->OnMatchStateChanged.RemoveAll(this);
		BlackoutGameState->OnMatchResultStateChanged.RemoveAll(this);
	}

	Super::BeginDestroy();
}

bool UBlackoutMatchResultWidgetController::Initialize(APlayerController* InPlayerController)
{
	if (!ResolveDependencies(InPlayerController))
	{
		return false;
	}

	BindCallbacksToDependencies();
	return true;
}

void UBlackoutMatchResultWidgetController::BindCallbacksToDependencies()
{
	ABlackoutGameState* BlackoutGameState = GameState.Get();
	if (!BlackoutGameState)
	{
		BO_LOG_CORE(Warning, "결과창 바인딩 보류: GameState가 유효하지 않습니다.");
		return;
	}

	if (!bGameStateCallbacksBound)
	{
		BlackoutGameState->OnPlayerArrayChanged.AddDynamic(
			this,
			&UBlackoutMatchResultWidgetController::HandlePlayerArrayChanged);
		BlackoutGameState->OnMatchStateChanged.AddDynamic(
			this,
			&UBlackoutMatchResultWidgetController::HandleMatchStateChanged);
		BlackoutGameState->OnMatchResultStateChanged.AddDynamic(
			this,
			&UBlackoutMatchResultWidgetController::HandleMatchResultStateChanged);
		bGameStateCallbacksBound = true;
	}

	RefreshResult();
}

void UBlackoutMatchResultWidgetController::BroadcastInitialResult()
{
	OnResultVisibilityChanged.Broadcast(IsResultVisible());
	RefreshResult();
	OnLocalConfirmStateChanged.Broadcast(bLocalPlayerConfirmed);
}

void UBlackoutMatchResultWidgetController::RefreshResult()
{
	TArray<FBlackoutMatchResultPlayerStatsData> PlayerStatsList;
	TSet<ABlackoutPlayerState*> VisiblePlayerStates;
	int32 DisplayOrder = 0;

	for (ABlackoutPlayerState* PlayerState : GetParticipantPlayerStates())
	{
		if (!PlayerState)
		{
			continue;
		}

		VisiblePlayerStates.Add(PlayerState);
		BindPlayer(PlayerState);
		PlayerStatsList.Add(BuildPlayerStatsData(PlayerState, DisplayOrder));
		++DisplayOrder;
	}

	TArray<ABlackoutPlayerState*> RemovedPlayerStates;
	for (const auto& BindingPair : PlayerBindings)
	{
		if (!VisiblePlayerStates.Contains(BindingPair.Key))
		{
			RemovedPlayerStates.Add(BindingPair.Key);
		}
	}

	for (ABlackoutPlayerState* RemovedPlayerState : RemovedPlayerStates)
	{
		UnbindPlayer(RemovedPlayerState);
	}

	OnResultRebuilt.Broadcast(BuildSummaryData(PlayerStatsList), PlayerStatsList);
}

void UBlackoutMatchResultWidgetController::RequestConfirmResult()
{
	if (!IsResultVisible() || bLocalPlayerConfirmed)
	{
		return;
	}

	bLocalPlayerConfirmed = true;

	if (ABlackoutPlayerState* BlackoutPlayerState = LocalPlayerState.Get())
	{
		ConfirmedPlayerStates.Add(BlackoutPlayerState);
		const int32 DisplayOrder = GetParticipantPlayerStates().IndexOfByKey(BlackoutPlayerState);
		OnPlayerStatsChanged.Broadcast(BuildPlayerStatsData(BlackoutPlayerState, DisplayOrder));
	}

	OnLocalConfirmStateChanged.Broadcast(true);
}

bool UBlackoutMatchResultWidgetController::ResolveDependencies(APlayerController* InPlayerController)
{
	if (!InPlayerController || !InPlayerController->IsLocalController())
	{
		BO_LOG_CORE(Error, "결과창 초기화 실패: 로컬 PlayerController가 유효하지 않습니다.");
		return false;
	}

	ABlackoutPlayerState* BlackoutPlayerState = InPlayerController->GetPlayerState<ABlackoutPlayerState>();
	if (!BlackoutPlayerState)
	{
		BO_LOG_CORE(Verbose, "결과창 초기화 보류: ABlackoutPlayerState 복제를 기다리는 중입니다.");
		return false;
	}

	ABlackoutGameState* BlackoutGameState =
		InPlayerController->GetWorld() ? InPlayerController->GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	if (!BlackoutGameState)
	{
		BO_LOG_CORE(Verbose, "결과창 초기화 보류: ABlackoutGameState 복제를 기다리는 중입니다.");
		return false;
	}

	if (GameState.Get() != BlackoutGameState)
	{
		if (ABlackoutGameState* PreviousGameState = GameState.Get())
		{
			PreviousGameState->OnPlayerArrayChanged.RemoveAll(this);
			PreviousGameState->OnMatchStateChanged.RemoveAll(this);
			PreviousGameState->OnMatchResultStateChanged.RemoveAll(this);
		}

		bGameStateCallbacksBound = false;
		UnbindAllPlayers();
	}

	PlayerController = InPlayerController;
	LocalPlayerState = BlackoutPlayerState;
	GameState = BlackoutGameState;
	return true;
}

void UBlackoutMatchResultWidgetController::BindPlayer(ABlackoutPlayerState* PlayerState)
{
	if (!PlayerState || PlayerBindings.Contains(PlayerState))
	{
		return;
	}

	FBlackoutMatchResultPlayerBinding Binding;
	Binding.PlayerState = PlayerState;

	const TWeakObjectPtr<ABlackoutPlayerState> WeakPlayerState(PlayerState);
	Binding.MatchStatsChangedHandle = PlayerState->OnMatchStatsChangedNative.AddLambda(
		[this, WeakPlayerState]()
		{
			if (ABlackoutPlayerState* BoundPlayerState = WeakPlayerState.Get())
			{
				BroadcastPlayerStats(BoundPlayerState);
			}
		});

	Binding.NameChangedHandle = PlayerState->OnPlayerNameChangedNative.AddLambda(
		[this, WeakPlayerState]()
		{
			if (ABlackoutPlayerState* BoundPlayerState = WeakPlayerState.Get())
			{
				BroadcastPlayerStats(BoundPlayerState);
			}
		});

	PlayerBindings.Add(PlayerState, Binding);
}

void UBlackoutMatchResultWidgetController::UnbindPlayer(ABlackoutPlayerState* PlayerState)
{
	FBlackoutMatchResultPlayerBinding Binding;
	if (!PlayerBindings.RemoveAndCopyValue(PlayerState, Binding))
	{
		return;
	}

	if (ABlackoutPlayerState* BoundPlayerState = Binding.PlayerState.Get())
	{
		if (Binding.MatchStatsChangedHandle.IsValid())
		{
			BoundPlayerState->OnMatchStatsChangedNative.Remove(Binding.MatchStatsChangedHandle);
		}
		if (Binding.NameChangedHandle.IsValid())
		{
			BoundPlayerState->OnPlayerNameChangedNative.Remove(Binding.NameChangedHandle);
		}
	}
}

void UBlackoutMatchResultWidgetController::UnbindAllPlayers()
{
	TArray<ABlackoutPlayerState*> BoundPlayerStates;
	PlayerBindings.GetKeys(BoundPlayerStates);

	for (ABlackoutPlayerState* BoundPlayerState : BoundPlayerStates)
	{
		UnbindPlayer(BoundPlayerState);
	}
}

void UBlackoutMatchResultWidgetController::BroadcastPlayerStats(ABlackoutPlayerState* PlayerState)
{
	if (!PlayerState || !PlayerBindings.Contains(PlayerState))
	{
		return;
	}

	const TArray<ABlackoutPlayerState*> ParticipantPlayerStates = GetParticipantPlayerStates();
	const int32 DisplayOrder = ParticipantPlayerStates.IndexOfByKey(PlayerState);
	OnPlayerStatsChanged.Broadcast(BuildPlayerStatsData(PlayerState, DisplayOrder));
}

FBlackoutMatchResultSummaryData UBlackoutMatchResultWidgetController::BuildSummaryData(
	const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList) const
{
	FBlackoutMatchResultSummaryData SummaryData;
	const ABlackoutGameState* BlackoutGameState = GameState.Get();
	SummaryData.DefeatedBossType = BlackoutGameState ? BlackoutGameState->DefeatedBossType : EBossType::Main;
	SummaryData.ResultTitle = SummaryData.DefeatedBossType == EBossType::Mid
		? FText::FromString(TEXT("AREA CLEARED"))
		: FText::FromString(TEXT("MISSION COMPLETE"));
	SummaryData.ResultSubtitle = FText::FromString(TEXT("Squad statistics"));
	SummaryData.RequiredConfirmPlayerCount = PlayerStatsList.Num();

	if (BlackoutGameState && BlackoutGameState->MatchResultAutoTravelServerTime > 0.f)
	{
		const float CurrentTime = BlackoutGameState->GetServerWorldTimeSeconds();
		SummaryData.AutoTravelRemainingTime = FMath::Max(
			0.f,
			BlackoutGameState->MatchResultAutoTravelServerTime - CurrentTime);
	}

	int32 TotalShotsFired = 0;
	int32 TotalShotsHit = 0;

	for (const FBlackoutMatchResultPlayerStatsData& PlayerStatsData : PlayerStatsList)
	{
		SummaryData.TotalDamageDealt += PlayerStatsData.DamageDealt;
		SummaryData.TotalKills += PlayerStatsData.Kills;
		SummaryData.TotalRevives += PlayerStatsData.Revives;
		TotalShotsFired += PlayerStatsData.ShotsFired;
		TotalShotsHit += PlayerStatsData.ShotsHit;

		if (PlayerStatsData.bHasConfirmedResult)
		{
			++SummaryData.ConfirmedPlayerCount;
		}
	}

	SummaryData.TeamAccuracyPercent = TotalShotsFired > 0
		? (static_cast<float>(TotalShotsHit) / static_cast<float>(TotalShotsFired)) * 100.0f
		: 0.0f;

	return SummaryData;
}

FBlackoutMatchResultPlayerStatsData UBlackoutMatchResultWidgetController::BuildPlayerStatsData(
	ABlackoutPlayerState* PlayerState,
	int32 DisplayOrder) const
{
	FBlackoutMatchResultPlayerStatsData PlayerStatsData;
	if (!PlayerState)
	{
		return PlayerStatsData;
	}

	const FBlackoutMatchStats& MatchStats = PlayerState->MatchStats;
	PlayerStatsData.PlayerState = PlayerState;
	PlayerStatsData.DisplayName = FText::FromString(PlayerState->GetPlayerName());
	PlayerStatsData.SelectedClassTag = PlayerState->SelectedClassTag;
	PlayerStatsData.DisplayOrder = DisplayOrder;
	PlayerStatsData.DamageDealt = MatchStats.DamageDealt;
	PlayerStatsData.Kills = MatchStats.Kills;
	PlayerStatsData.MeleeKills = MatchStats.MeleeKills;
	PlayerStatsData.ShotsFired = MatchStats.ShotsFired;
	PlayerStatsData.ShotsHit = MatchStats.ShotsHit;
	PlayerStatsData.AccuracyPercent = MatchStats.ShotsFired > 0
		? (static_cast<float>(MatchStats.ShotsHit) / static_cast<float>(MatchStats.ShotsFired)) * 100.0f
		: 0.0f;
	PlayerStatsData.ConsumablesUsed = MatchStats.ConsumablesUsed;
	PlayerStatsData.Revives = MatchStats.Revives;
	PlayerStatsData.bHasConfirmedResult = ConfirmedPlayerStates.Contains(PlayerState);
	PlayerStatsData.bIsLocalPlayer = PlayerState == LocalPlayerState.Get();
	PlayerStatsData.bIsValid = true;
	return PlayerStatsData;
}

TArray<ABlackoutPlayerState*> UBlackoutMatchResultWidgetController::GetParticipantPlayerStates() const
{
	TArray<ABlackoutPlayerState*> ParticipantPlayerStates;
	const ABlackoutGameState* BlackoutGameState = GameState.Get();
	if (!BlackoutGameState)
	{
		return ParticipantPlayerStates;
	}

	if (BlackoutGameState->MatchResultParticipants.Num() > 0)
	{
		ParticipantPlayerStates.Reserve(BlackoutGameState->MatchResultParticipants.Num());
		for (ABlackoutPlayerState* PlayerState : BlackoutGameState->MatchResultParticipants)
		{
			if (PlayerState)
			{
				ParticipantPlayerStates.Add(PlayerState);
			}
		}

		return ParticipantPlayerStates;
	}

	ParticipantPlayerStates.Reserve(BlackoutGameState->PlayerArray.Num());
	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		if (ABlackoutPlayerState* BlackoutPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase))
		{
			ParticipantPlayerStates.Add(BlackoutPlayerState);
		}
	}

	return ParticipantPlayerStates;
}

bool UBlackoutMatchResultWidgetController::IsResultVisible() const
{
	const ABlackoutGameState* BlackoutGameState = GameState.Get();
	return BlackoutGameState && BlackoutGameState->bIsMatchResultVisible;
}

void UBlackoutMatchResultWidgetController::HandlePlayerArrayChanged()
{
	RefreshResult();
}

void UBlackoutMatchResultWidgetController::HandleMatchStateChanged(EBlackoutMatchState)
{
	OnResultVisibilityChanged.Broadcast(IsResultVisible());
	RefreshResult();
}

void UBlackoutMatchResultWidgetController::HandleMatchResultStateChanged()
{
	const bool bIsVisible = IsResultVisible();
	if (!bIsVisible)
	{
		bLocalPlayerConfirmed = false;
		ConfirmedPlayerStates.Reset();
		OnLocalConfirmStateChanged.Broadcast(false);
	}

	OnResultVisibilityChanged.Broadcast(bIsVisible);
	RefreshResult();
}
