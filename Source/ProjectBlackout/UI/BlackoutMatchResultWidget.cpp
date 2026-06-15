#include "UI/BlackoutMatchResultWidget.h"

#include "Components/TextBlock.h"
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

	RefreshStatusText();
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
	RefreshStatusText();
}

void UBlackoutMatchResultWidget::UpdatePlayerStats(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData)
{
	if (StatsTableWidget)
	{
		StatsTableWidget->UpdatePlayerColumn(PlayerStatsData);
	}
}

void UBlackoutMatchResultWidget::NativeDestruct()
{
	UnbindWidgetControllerCallbacks();

	Super::NativeDestruct();
}

void UBlackoutMatchResultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetVisibility() != ESlateVisibility::Collapsed)
	{
		RefreshStatusText();
	}
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
}

void UBlackoutMatchResultWidget::HandleResultVisibilityChanged(bool bIsVisible)
{
	SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	RefreshStatusText();
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

void UBlackoutMatchResultWidget::RefreshStatusText()
{
	if (!StatusText)
	{
		return;
	}

	const float RemainingTime = WidgetController
		? WidgetController->GetAutoTravelRemainingTime()
		: SummaryData.AutoTravelRemainingTime;
	const bool bShouldShow = GetVisibility() != ESlateVisibility::Collapsed && RemainingTime > 0.f;
	if (!bShouldShow)
	{
		StatusText->SetText(FText::GetEmpty());
		StatusText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FText ResolvedStatusText = RemainingTime <= ImminentTravelTextThreshold
		? ImminentTravelText
		: FText::Format(
			AutoTravelCountdownFormatText,
			FText::AsNumber(FMath::CeilToInt(RemainingTime)));

	StatusText->SetText(ResolvedStatusText);
	StatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
}
