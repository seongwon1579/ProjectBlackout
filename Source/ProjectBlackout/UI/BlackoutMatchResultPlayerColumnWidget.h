#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutMatchResultTypes.h"
#include "BlackoutMatchResultPlayerColumnWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutMatchResultPlayerColumnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void SetPlayerStatsData(const FBlackoutMatchResultPlayerStatsData& InPlayerStatsData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Result")
	const FBlackoutMatchResultPlayerStatsData& GetPlayerStatsData() const { return PlayerStatsData; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultPlayerStatsData PlayerStatsData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> DamageDealtText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> KillsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> MeleeKillsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> AccuracyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> ShotsFiredText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> ShotsHitText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> ConsumablesUsedText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Result")
	TObjectPtr<UTextBlock> RevivesText;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Player Stats Data Changed"), Category = "Blackout|HUD|Result")
	void ReceivePlayerStatsDataChanged(const FBlackoutMatchResultPlayerStatsData& InPlayerStatsData);

private:
	static FText FormatInteger(int32 Value);
	static FText FormatPercent(float Value);
};
