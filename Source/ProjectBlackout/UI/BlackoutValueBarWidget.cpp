#include "UI/BlackoutValueBarWidget.h"

void UBlackoutValueBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyValue(InitialCurrentValue, InitialMaxValue, true);
}

void UBlackoutValueBarWidget::SetValue(float NewCurrentValue, float NewMaxValue)
{
	ApplyValue(NewCurrentValue, NewMaxValue, false);
}

void UBlackoutValueBarWidget::SetCurrentValue(float NewCurrentValue)
{
	ApplyValue(NewCurrentValue, MaxValue, false);
}

void UBlackoutValueBarWidget::SetMaxValue(float NewMaxValue)
{
	ApplyValue(CurrentValue, NewMaxValue, false);
}

void UBlackoutValueBarWidget::ApplyValue(float NewCurrentValue, float NewMaxValue, bool bForceBroadcast)
{
	const float SanitizedMaxValue = FMath::Max(NewMaxValue, 0.0f);
	const float SanitizedCurrentValue = SanitizedMaxValue > 0.0f
		? FMath::Clamp(NewCurrentValue, 0.0f, SanitizedMaxValue)
		: 0.0f;
	const float NewNormalizedValue = SanitizedMaxValue > 0.0f
		? FMath::Clamp(SanitizedCurrentValue / SanitizedMaxValue, 0.0f, 1.0f)
		: 0.0f;
	const bool bNewIsDepleted = NewNormalizedValue <= KINDA_SMALL_NUMBER;

	const bool bValueChanged = bForceBroadcast
		|| !FMath::IsNearlyEqual(CurrentValue, SanitizedCurrentValue)
		|| !FMath::IsNearlyEqual(MaxValue, SanitizedMaxValue)
		|| !FMath::IsNearlyEqual(NormalizedValue, NewNormalizedValue);
	const bool bDepletedChanged = bForceBroadcast || bIsDepleted != bNewIsDepleted;

	CurrentValue = SanitizedCurrentValue;
	MaxValue = SanitizedMaxValue;
	NormalizedValue = NewNormalizedValue;
	bIsDepleted = bNewIsDepleted;

	if (bValueChanged)
	{
		ReceiveValueChanged(CurrentValue, MaxValue, NormalizedValue);
	}

	if (bDepletedChanged)
	{
		ReceiveDepletedChanged(bIsDepleted);
	}
}
