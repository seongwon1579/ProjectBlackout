#include "UI/BlackoutInteractionPromptWidget.h"

#include "Components/TextBlock.h"

void UBlackoutInteractionPromptWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	FBlackoutInteractionPromptData PreviewPromptData = InteractionPromptData;
	
	if (IsDesignTime())
	{
		PreviewPromptData.bIsVisible = true;
		PreviewPromptData.bShowProgress = true;
		PreviewPromptData.ProgressNormalized = 0.66f;
		PreviewPromptData.PromptText = FText::FromString(TEXT("상호작용"));
	}

	SetInteractionPromptData(PreviewPromptData);
}

void UBlackoutInteractionPromptWidget::SetInteractionPromptData(const FBlackoutInteractionPromptData& InInteractionPromptData)
{
	InteractionPromptData = InInteractionPromptData;

	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;
	const bool bShowPrompt = InteractionPromptData.bIsVisible;
	const bool bShowPromptText = bShowPrompt && !InteractionPromptData.PromptText.IsEmpty();

	SetVisibility(bShowPrompt ? VisibleState : HiddenState);

	if (PromptText)
	{
		PromptText->SetText(InteractionPromptData.PromptText);
		PromptText->SetVisibility(bShowPromptText ? VisibleState : HiddenState);
	}

	ReceiveInteractionPromptChanged(InteractionPromptData);
}
