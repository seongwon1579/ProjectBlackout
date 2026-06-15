#include "UI/BlackoutMatchResultStatsTableWidget.h"

#include "Components/HorizontalBox.h"
#include "Core/BlackoutLog.h"
#include "Framework/BlackoutPlayerState.h"
#include "UI/BlackoutMatchResultPlayerColumnWidget.h"

void UBlackoutMatchResultStatsTableWidget::RebuildColumns(
	const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList)
{
	TSet<TObjectKey<ABlackoutPlayerState>> ValidPlayerStates;

	for (const FBlackoutMatchResultPlayerStatsData& PlayerStatsData : PlayerStatsList)
	{
		ABlackoutPlayerState* PlayerState = PlayerStatsData.PlayerState.Get();
		if (!PlayerState || !PlayerStatsData.bIsValid)
		{
			continue;
		}

		ValidPlayerStates.Add(PlayerState);
		UpdatePlayerColumn(PlayerStatsData);
	}

	RemoveMissingColumns(ValidPlayerStates);
	ReceiveColumnsRebuilt(PlayerStatsList);
}

void UBlackoutMatchResultStatsTableWidget::UpdatePlayerColumn(
	const FBlackoutMatchResultPlayerStatsData& PlayerStatsData)
{
	ABlackoutPlayerState* PlayerState = PlayerStatsData.PlayerState.Get();
	if (!PlayerState || !PlayerStatsData.bIsValid)
	{
		return;
	}

	const TObjectKey<ABlackoutPlayerState> PlayerStateKey(PlayerState);
	UBlackoutMatchResultPlayerColumnWidget* PlayerColumnWidget = nullptr;

	if (TWeakObjectPtr<UBlackoutMatchResultPlayerColumnWidget>* ExistingWidget =
		PlayerColumnWidgets.Find(PlayerStateKey))
	{
		PlayerColumnWidget = ExistingWidget->Get();
	}

	if (!PlayerColumnWidget)
	{
		PlayerColumnWidget = CreatePlayerColumn(PlayerStatsData);
		if (!PlayerColumnWidget)
		{
			return;
		}

		PlayerColumnWidgets.Add(PlayerStateKey, PlayerColumnWidget);
	}

	PlayerColumnWidget->SetPlayerStatsData(PlayerStatsData);
	ReceivePlayerColumnUpdated(PlayerStatsData);
}

void UBlackoutMatchResultStatsTableWidget::NativeDestruct()
{
	PlayerColumnWidgets.Reset();
	Super::NativeDestruct();
}

UBlackoutMatchResultPlayerColumnWidget* UBlackoutMatchResultStatsTableWidget::CreatePlayerColumn(
	const FBlackoutMatchResultPlayerStatsData& PlayerStatsData)
{
	if (!PlayerColumnWidgetClass)
	{
		BO_LOG_CORE(Warning, "결과창 플레이어 컬럼 생성 실패: PlayerColumnWidgetClass가 지정되지 않았습니다.");
		return nullptr;
	}

	if (!PlayerColumnContainer)
	{
		BO_LOG_CORE(Warning, "결과창 플레이어 컬럼 생성 실패: PlayerColumnContainer가 바인딩되지 않았습니다.");
		return nullptr;
	}

	UBlackoutMatchResultPlayerColumnWidget* PlayerColumnWidget =
		CreateWidget<UBlackoutMatchResultPlayerColumnWidget>(GetOwningPlayer(), PlayerColumnWidgetClass);
	if (!PlayerColumnWidget)
	{
		BO_LOG_CORE(Error, "결과창 플레이어 컬럼 생성 실패: CreateWidget이 nullptr을 반환했습니다.");
		return nullptr;
	}

	PlayerColumnContainer->AddChildToHorizontalBox(PlayerColumnWidget);
	PlayerColumnWidget->SetPlayerStatsData(PlayerStatsData);
	return PlayerColumnWidget;
}

void UBlackoutMatchResultStatsTableWidget::RemoveMissingColumns(
	const TSet<TObjectKey<ABlackoutPlayerState>>& ValidPlayerStates)
{
	for (auto It = PlayerColumnWidgets.CreateIterator(); It; ++It)
	{
		if (ValidPlayerStates.Contains(It.Key()))
		{
			continue;
		}

		if (UBlackoutMatchResultPlayerColumnWidget* PlayerColumnWidget = It.Value().Get())
		{
			PlayerColumnWidget->RemoveFromParent();
		}

		It.RemoveCurrent();
	}
}
