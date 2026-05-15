#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutInteractionPromptWidget.generated.h"

class UProgressBar;
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

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Revive", meta = (DeprecatedFunction, DeprecationMessage = "SetInteractionPromptData를 사용하세요."))
	void SetRevivePromptData(const FBlackoutInteractionPromptData& InRevivePromptData)
	{
		SetInteractionPromptData(InRevivePromptData);
	}

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Interaction")
	const FBlackoutInteractionPromptData& GetInteractionPromptData() const { return InteractionPromptData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Revive", meta = (DeprecatedFunction, DeprecationMessage = "GetInteractionPromptData를 사용하세요."))
	const FBlackoutInteractionPromptData& GetRevivePromptData() const { return InteractionPromptData; }

protected:
	virtual void NativePreConstruct() override;

	/** 비어 있는 WBP에서도 바로 확인할 수 있도록 기본 레이아웃을 생성합니다. */
	void EnsureDefaultLayout();

	/** 디자이너에서 수동으로 배치한 위젯을 이름 기준으로 다시 연결합니다. */
	void ResolveOptionalWidgetsFromTree();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FBlackoutInteractionPromptData InteractionPromptData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FLinearColor ErrorTextColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Interaction Prompt Changed"), Category = "Blackout|HUD|Interaction")
	void ReceiveInteractionPromptChanged(const FBlackoutInteractionPromptData& InInteractionPromptData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Revive Prompt Changed", DeprecatedFunction, DeprecationMessage = "On Interaction Prompt Changed를 사용하세요."), Category = "Blackout|HUD|Revive")
	void ReceiveRevivePromptChanged(const FBlackoutInteractionPromptData& InRevivePromptData);
};
