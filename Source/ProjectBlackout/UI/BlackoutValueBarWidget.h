// ─── 구현 내역 ───────────────────────
//  - 김민영: 체력·스태미나 값 바 위젯 및 수치 보간 표시 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutValueBarWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutValueBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetValue(float NewCurrentValue, float NewMaxValue);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetCurrentValue(float NewCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetMaxValue(float NewMaxValue);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetUseInterpolation(bool bInUseInterpolation) { bUseInterpolation = bInUseInterpolation; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetInterpolationSpeed(float InSpeed) { InterpolationSpeed = InSpeed; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetCurrentValue() const { return CurrentValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetMaxValue() const { return MaxValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetNormalizedValue() const { return NormalizedValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	bool IsDepleted() const { return bIsDepleted; }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar", meta = (ClampMin = "0.0"))
	float InitialCurrentValue = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar", meta = (ClampMin = "0.0"))
	float InitialMaxValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|ValueBar")
	bool bUseInterpolation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|ValueBar", meta = (EditCondition = "bUseInterpolation"))
	float InterpolationSpeed = 15.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar")
	float CurrentValue = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar")
	float MaxValue = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar")
	float NormalizedValue = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar")
	bool bIsDepleted = true;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Value Changed"), Category = "Blackout|HUD|ValueBar")
	void ReceiveValueChanged(float NewCurrentValue, float NewMaxValue, float NewNormalizedValue);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Depleted Changed"), Category = "Blackout|HUD|ValueBar")
	void ReceiveDepletedChanged(bool bNewIsDepleted);

private:
	void ApplyValue(float NewCurrentValue, float NewMaxValue, bool bForceBroadcast);

	float TargetCurrentValue = 0.0f;
	float TargetMaxValue = 0.0f;
	bool bHasInitializedTargets = false;
};
