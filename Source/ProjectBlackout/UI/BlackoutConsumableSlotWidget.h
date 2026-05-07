#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutConsumableTypes.h"
#include "BlackoutConsumableSlotWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutConsumableSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Consumable")
	void SetConsumableSlotData(const FBlackoutConsumableSlotData& NewSlotData);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Consumable")
	void SetConsumableCount(int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Consumable")
	void SetConsumableIcon(UTexture2D* NewIcon);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData GetConsumableSlotData() const { return SlotData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	int32 GetConsumableCount() const { return ConsumableCount; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Consumable")
	bool IsConsumableAvailable() const { return bIsConsumableAvailable; }

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	int32 ConsumableCount = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	bool bIsConsumableAvailable = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	FBlackoutConsumableSlotData SlotData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Consumable")
	TObjectPtr<UImage> IconImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Consumable")
	TObjectPtr<UTextBlock> CountText;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Slot Data Changed"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableSlotDataChanged(const FBlackoutConsumableSlotData& NewSlotData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Count Changed"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableCountChanged(int32 NewCount);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Availability Changed"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableAvailabilityChanged(bool bNewIsAvailable);
};
