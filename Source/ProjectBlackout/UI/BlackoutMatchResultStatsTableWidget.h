#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutMatchResultTypes.h"
#include "BlackoutMatchResultStatsTableWidget.generated.h"

class ABlackoutPlayerState;
class UBlackoutMatchResultPlayerColumnWidget;
class UHorizontalBox;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutMatchResultStatsTableWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void RebuildColumns(const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void UpdatePlayerColumn(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UHorizontalBox> PlayerColumnContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	TSubclassOf<UBlackoutMatchResultPlayerColumnWidget> PlayerColumnWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Result")
	TArray<FBlackoutMatchResultStatRowDefinition> StatRows;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Columns Rebuilt"), Category = "Blackout|HUD|Result")
	void ReceiveColumnsRebuilt(const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Player Column Updated"), Category = "Blackout|HUD|Result")
	void ReceivePlayerColumnUpdated(const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);

private:
	UBlackoutMatchResultPlayerColumnWidget* CreatePlayerColumn(
		const FBlackoutMatchResultPlayerStatsData& PlayerStatsData);
	void RemoveMissingColumns(const TSet<TObjectKey<ABlackoutPlayerState>>& ValidPlayerStates);

	TMap<TObjectKey<ABlackoutPlayerState>, TWeakObjectPtr<UBlackoutMatchResultPlayerColumnWidget>> PlayerColumnWidgets;
};
