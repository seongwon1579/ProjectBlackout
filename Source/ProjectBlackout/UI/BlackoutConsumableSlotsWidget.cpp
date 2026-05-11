#include "UI/BlackoutConsumableSlotsWidget.h"

#include "UI/BlackoutConsumableSlotWidget.h"

void UBlackoutConsumableSlotsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	BloodRootSlotData.CurrentCount = BloodRootCount;
	GulSerumSlotData.CurrentCount = GulSerumCount;
	SetConsumableSlotData(BloodRootSlotData, GulSerumSlotData);
}

void UBlackoutConsumableSlotsWidget::SetConsumableSlotData(
	const FBlackoutConsumableSlotData& NewBloodRootData,
	const FBlackoutConsumableSlotData& NewGulSerumData)
{
	BloodRootSlotData = NewBloodRootData;
	GulSerumSlotData = NewGulSerumData;

	BloodRootCount = FMath::Max(0, BloodRootSlotData.CurrentCount);
	GulSerumCount = FMath::Max(0, GulSerumSlotData.CurrentCount);
	BloodRootSlotData.CurrentCount = BloodRootCount;
	GulSerumSlotData.CurrentCount = GulSerumCount;

	if (BloodRootSlotWidget)
	{
		BloodRootSlotWidget->SetConsumableSlotData(BloodRootSlotData);
	}

	if (GulSerumSlotWidget)
	{
		GulSerumSlotWidget->SetConsumableSlotData(GulSerumSlotData);
	}

	ReceiveConsumableSlotDataChanged(BloodRootSlotData, GulSerumSlotData);
	ReceiveConsumableCountsChanged(BloodRootCount, GulSerumCount);
}

void UBlackoutConsumableSlotsWidget::SetConsumableCounts(int32 NewBloodRootCount, int32 NewGulSerumCount)
{
	BloodRootCount = FMath::Max(0, NewBloodRootCount);
	GulSerumCount = FMath::Max(0, NewGulSerumCount);
	BloodRootSlotData.CurrentCount = BloodRootCount;
	GulSerumSlotData.CurrentCount = GulSerumCount;

	if (BloodRootSlotWidget)
	{
		BloodRootSlotWidget->SetConsumableSlotData(BloodRootSlotData);
	}

	if (GulSerumSlotWidget)
	{
		GulSerumSlotWidget->SetConsumableSlotData(GulSerumSlotData);
	}

	ReceiveConsumableCountsChanged(BloodRootCount, GulSerumCount);
}
