#include "UI/BlackoutPartyMemberStatusWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "UI/BlackoutValueBarWidget.h"

void UBlackoutPartyMemberStatusWidget::SetStatusData(const FBlackoutPartyMemberStatusData& InStatusData)
{
	const bool bWasDowned = StatusData.bIsDowned;
	StatusData = InStatusData;

	if (PlayerNameText)
	{
		PlayerNameText->SetText(StatusData.DisplayName);
	}

	if (HealthBarWidget)
	{
		const float DisplayHealth = StatusData.bIsDowned ? 0.0f : StatusData.CurrentHealth;
		HealthBarWidget->SetValue(DisplayHealth, StatusData.MaxHealth);
	}

	const ESlateVisibility DownedVisibility = StatusData.bIsDowned
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed;

	if (DownedIconWidget)
	{
		DownedIconWidget->SetVisibility(DownedVisibility);
	}

	if (ReviveTextWidget)
	{
		ReviveTextWidget->SetVisibility(DownedVisibility);
		if (StatusData.bIsReviveInteractionActive)
		{
			ReviveTextWidget->SetText(RevivingStatusText);
			ReviveTextWidget->SetColorAndOpacity(RevivingStatusTextColor);
		}
		else
		{
			ReviveTextWidget->SetText(StatusData.bIsDowned ? DownedStatusText : FText::GetEmpty());
			ReviveTextWidget->SetColorAndOpacity(StatusData.bIsDowned ? DownedStatusTextColor : DefaultStatusTextColor);
		}
	}

	ReceiveStatusDataChanged(StatusData);

	if (bWasDowned != StatusData.bIsDowned)
	{
		ReceiveDownedStateChanged(StatusData.bIsDowned);
	}
}
