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

	// 전달받은 쿨다운 시간 정보 캐싱
	CooldownRemaining = SlotData.CooldownRemaining;
	CooldownDuration = SlotData.CooldownDuration;

	// 수량이 있고 쿨다운 상태가 아닐 때만 소모품 사용 가능으로 평가
	bIsConsumableAvailable = (ConsumableCount > 0) && (CooldownRemaining <= 0.0f);

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
	ReceiveConsumableCooldownUpdated(CooldownRemaining, CooldownDuration);
}

void UBlackoutConsumableSlotWidget::SetConsumableCount(int32 NewCount)
{
	const bool bWasAvailable = bIsConsumableAvailable;

	ConsumableCount = FMath::Max(0, NewCount);
	SlotData.CurrentCount = ConsumableCount;

	// 수량 감소/증가 시점에도 쿨다운 잔여 여부를 함께 반영하여 평가
	bIsConsumableAvailable = (ConsumableCount > 0) && (CooldownRemaining <= 0.0f);

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

void UBlackoutConsumableSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 쿨다운이 진행 중일 때 매 프레임 감산 연산 수행
	if (CooldownRemaining > 0.0f)
	{
		CooldownRemaining = FMath::Max(0.0f, CooldownRemaining - InDeltaTime);

		// UMG 블루프린트 연출용 이벤트 호출
		ReceiveConsumableCooldownUpdated(CooldownRemaining, CooldownDuration);

		// 쿨다운이 방금 끝난 시점의 상태 전환 처리
		if (CooldownRemaining <= 0.0f)
		{
			const bool bWasAvailable = bIsConsumableAvailable;
			bIsConsumableAvailable = ConsumableCount > 0;

			if (bWasAvailable != bIsConsumableAvailable)
			{
				ReceiveConsumableAvailabilityChanged(bIsConsumableAvailable);
			}
		}
	}
}
