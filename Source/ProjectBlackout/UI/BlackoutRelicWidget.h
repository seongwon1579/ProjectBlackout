// ─── 구현 내역 ───────────────────────
//  - 김민영: 유물 HUD 카운터 바인딩 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutRelicWidget.generated.h"

class UTextBlock;

/**
 * HUD에서 유물 충전 횟수를 표시하는 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutRelicWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Relic")
	void SetRelicCharges(int32 NewCurrentCharges, int32 NewMaxCharges);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Relic")
	int32 GetCurrentCharges() const { return CurrentCharges; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Relic")
	int32 GetMaxCharges() const { return MaxCharges; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Relic")
	bool HasRelicCharge() const { return bHasRelicCharge; }

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Relic")
	int32 CurrentCharges = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Relic")
	int32 MaxCharges = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Relic")
	bool bHasRelicCharge = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Relic")
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Relic")
	TObjectPtr<UTextBlock> MaxCountText;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Relic Charges Changed"), Category = "Blackout|HUD|Relic")
	void ReceiveRelicChargesChanged(int32 NewCurrentCharges, int32 NewMaxCharges);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Relic Availability Changed"), Category = "Blackout|HUD|Relic")
	void ReceiveRelicAvailabilityChanged(bool bNewHasRelicCharge);
};
