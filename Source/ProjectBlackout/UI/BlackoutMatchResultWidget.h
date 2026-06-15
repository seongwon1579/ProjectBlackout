#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutMatchResultTypes.h"
#include "BlackoutMatchResultWidget.generated.h"

class UBlackoutMatchResultStatsTableWidget;
class UBlackoutMatchResultWidgetController;
class UButton;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutMatchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void SetWidgetController(UBlackoutMatchResultWidgetController* InWidgetController);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void RebuildResult(
		const FBlackoutMatchResultSummaryData& InSummaryData,
		const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void UpdatePlayerStats(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void SetConfirmButtonEnabled(bool bEnabled);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UBlackoutMatchResultStatsTableWidget> StatsTableWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	TObjectPtr<UBlackoutMatchResultWidgetController> WidgetController;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultSummaryData SummaryData;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Result Visibility Changed"), Category = "Blackout|HUD|Result")
	void ReceiveResultVisibilityChanged(bool bIsVisible);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Result Rebuilt"), Category = "Blackout|HUD|Result")
	void ReceiveResultRebuilt(
		const FBlackoutMatchResultSummaryData& InSummaryData,
		const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Confirm State Changed"), Category = "Blackout|HUD|Result")
	void ReceiveConfirmStateChanged(bool bHasConfirmed);

private:
	void UnbindWidgetControllerCallbacks();

	UFUNCTION()
	void HandleResultVisibilityChanged(bool bIsVisible);

	UFUNCTION()
	void HandleResultRebuilt(
		const FBlackoutMatchResultSummaryData& InSummaryData,
		const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION()
	void HandlePlayerStatsChanged(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);

	UFUNCTION()
	void HandleLocalConfirmStateChanged(bool bHasConfirmed);

	UFUNCTION()
	void HandleConfirmClicked();
};
