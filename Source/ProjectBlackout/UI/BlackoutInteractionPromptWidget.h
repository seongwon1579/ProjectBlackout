#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutInteractionPromptWidget.generated.h"

class UTextBlock;

/**
 * 월드에 붙는 상호작용 프롬프트를 표시하는 공용 HUD 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Interaction")
	void SetInteractionPromptData(const FBlackoutInteractionPromptData& InInteractionPromptData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Interaction")
	const FBlackoutInteractionPromptData& GetInteractionPromptData() const { return InteractionPromptData; }

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FBlackoutInteractionPromptData InteractionPromptData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UTextBlock> PromptText;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Interaction Prompt Changed"), Category = "Blackout|HUD|Interaction")
	void ReceiveInteractionPromptChanged(const FBlackoutInteractionPromptData& InInteractionPromptData);
};
