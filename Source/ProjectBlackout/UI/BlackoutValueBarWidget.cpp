#include "UI/BlackoutValueBarWidget.h"

void UBlackoutValueBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyValue(InitialCurrentValue, InitialMaxValue, true);
}

void UBlackoutValueBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bUseInterpolation && bHasInitializedTargets)
	{
		const float NewCurrentValue = FMath::FInterpTo(CurrentValue, TargetCurrentValue, InDeltaTime, InterpolationSpeed);
		const float NewMaxValue = FMath::FInterpTo(MaxValue, TargetMaxValue, InDeltaTime, InterpolationSpeed);

		const float NewNormalizedValue = NewMaxValue > 0.0f
			? FMath::Clamp(NewCurrentValue / NewMaxValue, 0.0f, 1.0f)
			: 0.0f;
		const bool bNewIsDepleted = NewNormalizedValue <= KINDA_SMALL_NUMBER;

		const bool bValueChanged = !FMath::IsNearlyEqual(CurrentValue, NewCurrentValue)
			|| !FMath::IsNearlyEqual(MaxValue, NewMaxValue)
			|| !FMath::IsNearlyEqual(NormalizedValue, NewNormalizedValue);
		const bool bDepletedChanged = bIsDepleted != bNewIsDepleted;

		if (bValueChanged || bDepletedChanged)
		{
			CurrentValue = NewCurrentValue;
			MaxValue = NewMaxValue;
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
	}
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

	TargetCurrentValue = SanitizedCurrentValue;
	TargetMaxValue = SanitizedMaxValue;

	if (bForceBroadcast || !bUseInterpolation)
	{
		CurrentValue = TargetCurrentValue;
		MaxValue = TargetMaxValue;

		const float NewNormalizedValue = MaxValue > 0.0f
			? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f)
			: 0.0f;
		const bool bNewIsDepleted = NewNormalizedValue <= KINDA_SMALL_NUMBER;

		const bool bValueChanged = bForceBroadcast
			|| !FMath::IsNearlyEqual(NormalizedValue, NewNormalizedValue);
		const bool bDepletedChanged = bForceBroadcast || bIsDepleted != bNewIsDepleted;

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

		bHasInitializedTargets = true;
	}
	else
	{
		if (!bHasInitializedTargets)
		{
			CurrentValue = TargetCurrentValue;
			MaxValue = TargetMaxValue;
			NormalizedValue = MaxValue > 0.0f ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;
			bIsDepleted = NormalizedValue <= KINDA_SMALL_NUMBER;

			ReceiveValueChanged(CurrentValue, MaxValue, NormalizedValue);
			ReceiveDepletedChanged(bIsDepleted);
			bHasInitializedTargets = true;
		}
	}
}
