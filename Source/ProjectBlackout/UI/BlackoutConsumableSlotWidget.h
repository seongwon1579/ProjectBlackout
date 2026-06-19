// ─── 구현 내역 ───────────────────────
//  - 김민영: HUD 소모품 슬롯 위젯·아이콘·쿨다운 표시 및 GAS 연동 구현
// ──────────────────────────────────────

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

	// 현재 매 틱마다 로컬로 감소하는 쿨다운 남은 시간 (초)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	float CooldownRemaining = 0.0f;

	// 총 쿨다운 지속시간 (초)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Consumable")
	float CooldownDuration = 0.0f;

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

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 매 틱마다 감소하는 로컬 쿨다운 상태를 UMG 블루프린트(텍스트, 마스크 연출)에 통보하는 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Cooldown Updated"), Category = "Blackout|HUD|Consumable")
	void ReceiveConsumableCooldownUpdated(float RemainingTime, float Duration);
};
