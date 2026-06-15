#include "UI/BlackoutMatchResultWidget.h"

#include "Components/Button.h"
#include "Core/BlackoutLog.h"
#include "UI/BlackoutMatchResultStatsTableWidget.h"
#include "UI/BlackoutMatchResultWidgetController.h"

void UBlackoutMatchResultWidget::SetWidgetController(UBlackoutMatchResultWidgetController* InWidgetController)
{
	if (!InWidgetController)
	{
		BO_LOG_CORE(Warning, "결과창 컨트롤러 연결 실패: WidgetController가 유효하지 않습니다.");
		return;
	}

	UnbindWidgetControllerCallbacks();
	WidgetController = InWidgetController;

	WidgetController->OnResultVisibilityChanged.AddDynamic(
		this,
		&UBlackoutMatchResultWidget::HandleResultVisibilityChanged);
	WidgetController->OnResultRebuilt.AddDynamic(
		this,
		&UBlackoutMatchResultWidget::HandleResultRebuilt);
	WidgetController->OnPlayerStatsChanged.AddDynamic(
		this,
		&UBlackoutMatchResultWidget::HandlePlayerStatsChanged);
	WidgetController->OnLocalConfirmStateChanged.AddDynamic(
		this,
		&UBlackoutMatchResultWidget::HandleLocalConfirmStateChanged);

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UBlackoutMatchResultWidget::HandleConfirmClicked);
		ConfirmButton->OnClicked.AddDynamic(this, &UBlackoutMatchResultWidget::HandleConfirmClicked);
	}

	ReceiveConfirmStateChanged(false);
}

void UBlackoutMatchResultWidget::RebuildResult(
	const FBlackoutMatchResultSummaryData& InSummaryData,
	const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList)
{
	SummaryData = InSummaryData;

	if (StatsTableWidget)
	{
		StatsTableWidget->RebuildColumns(PlayerStatsList);
	}

	ReceiveResultRebuilt(SummaryData, PlayerStatsList);
}

void UBlackoutMatchResultWidget::UpdatePlayerStats(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData)
{
	if (StatsTableWidget)
	{
		StatsTableWidget->UpdatePlayerColumn(PlayerStatsData);
	}
}

void UBlackoutMatchResultWidget::SetConfirmButtonEnabled(bool bEnabled)
{
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(bEnabled);
	}
}

void UBlackoutMatchResultWidget::NativeDestruct()
{
	UnbindWidgetControllerCallbacks();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UBlackoutMatchResultWidget::HandleConfirmClicked);
	}

	Super::NativeDestruct();
}

void UBlackoutMatchResultWidget::UnbindWidgetControllerCallbacks()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnResultVisibilityChanged.RemoveAll(this);
	WidgetController->OnResultRebuilt.RemoveAll(this);
	WidgetController->OnPlayerStatsChanged.RemoveAll(this);
	WidgetController->OnLocalConfirmStateChanged.RemoveAll(this);
}

void UBlackoutMatchResultWidget::HandleResultVisibilityChanged(bool bIsVisible)
{
	SetVisibility(bIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	ReceiveResultVisibilityChanged(bIsVisible);
}

void UBlackoutMatchResultWidget::HandleResultRebuilt(
	const FBlackoutMatchResultSummaryData& InSummaryData,
	const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList)
{
	RebuildResult(InSummaryData, PlayerStatsList);
}

void UBlackoutMatchResultWidget::HandlePlayerStatsChanged(
	const FBlackoutMatchResultPlayerStatsData& PlayerStatsData)
{
	UpdatePlayerStats(PlayerStatsData);
}

void UBlackoutMatchResultWidget::HandleLocalConfirmStateChanged(bool bHasConfirmed)
{
	SetConfirmButtonEnabled(!bHasConfirmed);
	ReceiveConfirmStateChanged(bHasConfirmed);
}

void UBlackoutMatchResultWidget::HandleConfirmClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestConfirmResult();
	}
}
