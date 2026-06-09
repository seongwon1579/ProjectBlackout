#include "UI/BlackoutInteractionPromptWidget.h"

void UBlackoutInteractionPromptWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	FBlackoutInteractionPromptData PreviewPromptData = InteractionPromptData;
	
	if (IsDesignTime())
	{
		PreviewPromptData.bIsVisible = true;
		PreviewPromptData.bShowProgress = true;
		PreviewPromptData.ProgressNormalized = 0.66f;
	}

	SetInteractionPromptData(PreviewPromptData);
}

void UBlackoutInteractionPromptWidget::SetInteractionPromptData(const FBlackoutInteractionPromptData& InInteractionPromptData)
{
	InteractionPromptData = InInteractionPromptData;

	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;
	const bool bShowPrompt = InteractionPromptData.bIsVisible;

	SetVisibility(bShowPrompt ? VisibleState : HiddenState);

	ReceiveInteractionPromptChanged(InteractionPromptData);
}
