// ─── 구현 내역 ───────────────────────
//  - 허혁: 부활 진행 전용 위젯 분리 및 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutReviveProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 실제 부활 진행 중일 때 화면에 고정된 진행 UI를 표시하는 전용 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutReviveProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Interaction")
	void SetInteractionProgressData(const FBlackoutInteractionPromptData& InInteractionPromptData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Interaction")
	const FBlackoutInteractionPromptData& GetInteractionPromptData() const { return InteractionPromptData; }

protected:
	virtual void NativePreConstruct() override;

	/** 비어 있는 WBP에서도 즉시 확인할 수 있도록 기본 레이아웃을 만듭니다. */
	void EnsureDefaultLayout();

	/** 디자이너에서 수동 배치한 위젯을 이름 기준으로 다시 연결합니다. */
	void ResolveOptionalWidgetsFromTree();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FBlackoutInteractionPromptData InteractionPromptData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Interaction")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Interaction")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Interaction Progress Changed"), Category = "Blackout|HUD|Interaction")
	void ReceiveInteractionProgressChanged(const FBlackoutInteractionPromptData& InInteractionPromptData);
};
