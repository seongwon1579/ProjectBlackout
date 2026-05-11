#include "UI/BlackoutConsumableSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBlackoutConsumableSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SlotData.CurrentCount = ConsumableCount;
	SetConsumableSlotData(SlotData);
}

void UBlackoutConsumableSlotWidget::SetConsumableSlotData(const FBlackoutConsumableSlotData& NewSlotData)
{
	SlotData = NewSlotData;
	ConsumableCount = FMath::Max(0, SlotData.CurrentCount);
	SlotData.CurrentCount = ConsumableCount;
	bIsConsumableAvailable = ConsumableCount > 0;

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(SlotData.Icon);
	}

	if (CountText)
	{
		CountText->SetText(FText::AsNumber(ConsumableCount));
	}

	ReceiveConsumableSlotDataChanged(SlotData);
	ReceiveConsumableCountChanged(ConsumableCount);
	ReceiveConsumableAvailabilityChanged(bIsConsumableAvailable);
}

void UBlackoutConsumableSlotWidget::SetConsumableCount(int32 NewCount)
{
	const bool bWasAvailable = bIsConsumableAvailable;

	ConsumableCount = FMath::Max(0, NewCount);
	SlotData.CurrentCount = ConsumableCount;
	bIsConsumableAvailable = ConsumableCount > 0;

	if (CountText)
	{
		CountText->SetText(FText::AsNumber(ConsumableCount));
	}

	ReceiveConsumableCountChanged(ConsumableCount);

	if (bWasAvailable != bIsConsumableAvailable)
	{
		ReceiveConsumableAvailabilityChanged(bIsConsumableAvailable);
	}
}

void UBlackoutConsumableSlotWidget::SetConsumableIcon(UTexture2D* NewIcon)
{
	SlotData.Icon = NewIcon;

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(SlotData.Icon);
	}
}
