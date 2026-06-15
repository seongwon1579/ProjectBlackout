#include "UI/BlackoutMatchResultPlayerColumnWidget.h"

#include "Components/TextBlock.h"

void UBlackoutMatchResultPlayerColumnWidget::SetPlayerStatsData(
	const FBlackoutMatchResultPlayerStatsData& InPlayerStatsData)
{
	PlayerStatsData = InPlayerStatsData;

	if (PlayerNameText)
	{
		PlayerNameText->SetText(PlayerStatsData.DisplayName);
	}
	if (DamageDealtText)
	{
		DamageDealtText->SetText(FormatInteger(PlayerStatsData.DamageDealt));
	}
	if (KillsText)
	{
		KillsText->SetText(FormatInteger(PlayerStatsData.Kills));
	}
	if (MeleeKillsText)
	{
		MeleeKillsText->SetText(FormatInteger(PlayerStatsData.MeleeKills));
	}
	if (AccuracyText)
	{
		AccuracyText->SetText(FormatPercent(PlayerStatsData.AccuracyPercent));
	}
	if (ShotsFiredText)
	{
		ShotsFiredText->SetText(FormatInteger(PlayerStatsData.ShotsFired));
	}
	if (ShotsHitText)
	{
		ShotsHitText->SetText(FormatInteger(PlayerStatsData.ShotsHit));
	}
	if (ConsumablesUsedText)
	{
		ConsumablesUsedText->SetText(FormatInteger(PlayerStatsData.ConsumablesUsed));
	}
	if (RevivesText)
	{
		RevivesText->SetText(FormatInteger(PlayerStatsData.Revives));
	}

	ReceivePlayerStatsDataChanged(PlayerStatsData);
}

FText UBlackoutMatchResultPlayerColumnWidget::FormatInteger(int32 Value)
{
	return FText::AsNumber(Value);
}

FText UBlackoutMatchResultPlayerColumnWidget::FormatPercent(float Value)
{
	return FText::FromString(FString::Printf(TEXT("%.1f%%"), Value));
}
