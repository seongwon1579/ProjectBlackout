#include "UI/BlackoutDownedStateWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBlackoutDownedStateWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ResolveOptionalWidgetsFromTree();
}

void UBlackoutDownedStateWidget::SetDownedStateHUDData(const FBlackoutDownedStateHUDData& InHUDData)
{
	HUDData = InHUDData;

	ResolveOptionalWidgetsFromTree();

	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;

	SetVisibility(HUDData.bIsVisible ? VisibleState : HiddenState);

	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(1 - HUDData.ProgressNormalized, 0.0f, 1.0f));
		ProgressBar->SetVisibility(HUDData.bIsVisible ? VisibleState : HiddenState);
	}

	if (StatusText)
	{
		// 컨트롤러가 동적 문구를 보낸 경우엔 그것을 우선 사용하고, 비었다면 위젯 설정값으로 폴백합니다.
		const FText ResolvedText = HUDData.StatusText.IsEmpty()
			? ResolveStatusTextForMode(HUDData.HUDMode)
			: HUDData.StatusText;

		StatusText->SetText(ResolvedText);
		StatusText->SetColorAndOpacity(FSlateColor(DefaultTextColor));
		StatusText->SetVisibility(HUDData.bIsVisible && !ResolvedText.IsEmpty() ? VisibleState : HiddenState);
	}

	ReceiveDownedStateHUDDataChanged(HUDData);
}

FText UBlackoutDownedStateWidget::ResolveStatusTextForMode(EBlackoutHUDMode HUDMode) const
{
	switch (HUDMode)
	{
	case EBlackoutHUDMode::DownedDeathTimer:
		return DeathTimerStatusText;
	case EBlackoutHUDMode::DownedReviveTimer:
		return ReviveTimerStatusText;
	case EBlackoutHUDMode::Spectator:
		return SpectatorStatusText;
	case EBlackoutHUDMode::Combat:
	default:
		return FText::GetEmpty();
	}
}

void UBlackoutDownedStateWidget::ResolveOptionalWidgetsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!ProgressBar)
	{
		ProgressBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("ProgressBar")));
	}

	if (!StatusText)
	{
		StatusText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("StatusText")));
	}
}
