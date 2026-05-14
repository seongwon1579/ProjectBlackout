#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutRevivePromptWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 다운된 아군 부활 상호작용 프롬프트를 표시하는 전용 HUD 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutRevivePromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Revive")
	void SetRevivePromptData(const FBlackoutRevivePromptData& InRevivePromptData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Revive")
	const FBlackoutRevivePromptData& GetRevivePromptData() const { return RevivePromptData; }

protected:
	virtual void NativePreConstruct() override;

	/** 비어 있는 WBP에서도 바로 확인할 수 있도록 기본 레이아웃을 생성합니다. */
	void EnsureDefaultLayout();

	/** 디자이너에서 수동으로 배치한 위젯을 이름 기준으로 다시 연결합니다. */
	void ResolveOptionalWidgetsFromTree();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Revive")
	FBlackoutRevivePromptData RevivePromptData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Revive")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Revive")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Revive")
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Revive")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Revive")
	FLinearColor ErrorTextColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Revive Prompt Changed"), Category = "Blackout|HUD|Revive")
	void ReceiveRevivePromptChanged(const FBlackoutRevivePromptData& InRevivePromptData);
};
