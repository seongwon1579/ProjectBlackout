#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutValueBarWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutValueBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetValue(float NewCurrentValue, float NewMaxValue);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetCurrentValue(float NewCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|ValueBar")
	void SetMaxValue(float NewMaxValue);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetCurrentValue() const { return CurrentValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetMaxValue() const { return MaxValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	float GetNormalizedValue() const { return NormalizedValue; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|ValueBar")
	bool IsDepleted() const { return bIsDepleted; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar", meta = (ClampMin = "0.0"))
	float InitialCurrentValue = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|ValueBar", meta = (ClampMin = "0.0"))
	float InitialMaxValue = 1.0f;

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
};
