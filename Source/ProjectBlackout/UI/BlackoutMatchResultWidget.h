// ─── 구현 내역 ───────────────────────
//  - 김민영: 게임 결과 통계 화면 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutMatchResultTypes.h"
#include "BlackoutMatchResultWidget.generated.h"

class UBlackoutMatchResultStatsTableWidget;
class UBlackoutMatchResultWidgetController;
class UTextBlock;

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

protected:
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UBlackoutMatchResultStatsTableWidget> StatsTableWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	TObjectPtr<UBlackoutMatchResultWidgetController> WidgetController;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultSummaryData SummaryData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Result|Text")
	FText AutoTravelCountdownFormatText = FText::FromString(TEXT("{0}초 후 자동으로 이동..."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Result|Text")
	FText ImminentTravelText = FText::FromString(TEXT("잠시 후 이동합니다..."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Result|Text", meta = (ClampMin = "0.0"))
	float ImminentTravelTextThreshold = 1.0f;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Result Visibility Changed"), Category = "Blackout|HUD|Result")
	void ReceiveResultVisibilityChanged(bool bIsVisible);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Result Rebuilt"), Category = "Blackout|HUD|Result")
	void ReceiveResultRebuilt(
		const FBlackoutMatchResultSummaryData& InSummaryData,
		const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

private:
	void UnbindWidgetControllerCallbacks();
	void RefreshStatusText();

	UFUNCTION()
	void HandleResultVisibilityChanged(bool bIsVisible);

	UFUNCTION()
	void HandleResultRebuilt(
		const FBlackoutMatchResultSummaryData& InSummaryData,
		const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION()
	void HandlePlayerStatsChanged(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);
};
