// ─── 구현 내역 ───────────────────────
//  - 김민영: HUD 소모품 슬롯 묶음 위젯 및 아이콘 반영 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutConsumableTypes.h"
#include "BlackoutConsumableSlotsWidget.generated.h"

class UBlackoutConsumableSlotWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutConsumableSlotsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Consumable")
	void SetConsumableSlotData(
		const FBlackoutConsumableSlotData& NewBloodRootData,
		const FBlackoutConsumableSlotData& NewGulSerumData);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Consumable")
	void SetConsumableCounts(int32 NewBloodRootCount, int32 NewGulSerumCount);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData GetBloodRootSlotData() const { return BloodRootSlotData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData GetGulSerumSlotData() const { return GulSerumSlotData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	int32 GetBloodRootCount() const { return BloodRootCount; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	int32 GetGulSerumCount() const { return GulSerumCount; }

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Consumable")
	TObjectPtr<UBlackoutConsumableSlotWidget> BloodRootSlotWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Consumable")
	TObjectPtr<UBlackoutConsumableSlotWidget> GulSerumSlotWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	int32 BloodRootCount = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	int32 GulSerumCount = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData BloodRootSlotData;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData GulSerumSlotData;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Slot Data Changed"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableSlotDataChanged(
		const FBlackoutConsumableSlotData& NewBloodRootData,
		const FBlackoutConsumableSlotData& NewGulSerumData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Counts Changed"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableCountsChanged(int32 NewBloodRootCount, int32 NewGulSerumCount);
};
