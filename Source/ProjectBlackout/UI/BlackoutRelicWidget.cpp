#include "UI/BlackoutRelicWidget.h"

#include "Components/TextBlock.h"

void UBlackoutRelicWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetRelicCharges(CurrentCharges, MaxCharges);
}

void UBlackoutRelicWidget::SetRelicCharges(int32 NewCurrentCharges, int32 NewMaxCharges)
{
	const bool bHadRelicCharge = bHasRelicCharge;

	MaxCharges = FMath::Max(0, NewMaxCharges);
	CurrentCharges = FMath::Clamp(NewCurrentCharges, 0, MaxCharges);
	bHasRelicCharge = CurrentCharges > 0;

	if (CountText)
	{
		CountText->SetText(FText::AsNumber(CurrentCharges));
	}

	if (MaxCountText)
	{
		MaxCountText->SetText(FText::AsNumber(MaxCharges));
	}

	ReceiveRelicChargesChanged(CurrentCharges, MaxCharges);

	if (bHadRelicCharge != bHasRelicCharge)
	{
		ReceiveRelicAvailabilityChanged(bHasRelicCharge);
	}
}
