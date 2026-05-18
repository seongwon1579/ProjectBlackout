#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutPartyTypes.h"
#include "BlackoutPartyMemberStatusWidget.generated.h"

class UBlackoutValueBarWidget;
class UTextBlock;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutPartyMemberStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void SetStatusData(const FBlackoutPartyMemberStatusData& InStatusData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Party")
	const FBlackoutPartyMemberStatusData& GetStatusData() const { return StatusData; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FBlackoutPartyMemberStatusData StatusData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UBlackoutValueBarWidget> HealthBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UWidget> DownedIconWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UTextBlock> ReviveTextWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText DownedStatusText = FText::FromString(TEXT("쓰러짐"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText RevivingStatusText = FText::FromString(TEXT("부활 중"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor DefaultStatusTextColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor DownedStatusTextColor = FSlateColor(FLinearColor(1.0f, 0.15f, 0.1f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor RevivingStatusTextColor = FSlateColor(FLinearColor(0.2f, 0.75f, 1.0f, 1.0f));

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Status Data Changed"), Category = "Blackout|HUD|Party")
	void ReceiveStatusDataChanged(const FBlackoutPartyMemberStatusData& InStatusData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Downed State Changed"), Category = "Blackout|HUD|Party")
	void ReceiveDownedStateChanged(bool bIsDowned);
};
