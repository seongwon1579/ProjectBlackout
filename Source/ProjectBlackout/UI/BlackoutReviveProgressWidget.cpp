#include "UI/BlackoutReviveProgressWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBlackoutReviveProgressWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ResolveOptionalWidgetsFromTree();
	EnsureDefaultLayout();
	ResolveOptionalWidgetsFromTree();

	FBlackoutInteractionPromptData PreviewPromptData = InteractionPromptData;
	
	if (IsDesignTime())
	{
		PreviewPromptData.bIsVisible = true;
		PreviewPromptData.bShowProgress = true;
		PreviewPromptData.ProgressNormalized = 0.66f;
		PreviewPromptData.State = EBlackoutInteractionPromptState::InProgress;
	}

	SetInteractionProgressData(PreviewPromptData);
}

void UBlackoutReviveProgressWidget::EnsureDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InteractionProgressRoot"));
	if (!RootBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootBox;

	USizeBox* ProgressBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ProgressBox"));
	ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
	if (ProgressBox && ProgressBar)
	{
		ProgressBox->SetWidthOverride(180.0f);
		ProgressBox->SetHeightOverride(10.0f);
		ProgressBar->SetFillColorAndOpacity(FLinearColor(0.08f, 0.9f, 0.5f, 1.0f));
		ProgressBar->SetPercent(0.0f);
		ProgressBox->AddChild(ProgressBar);

		if (UVerticalBoxSlot* ProgressSlot = RootBox->AddChildToVerticalBox(ProgressBox))
		{
			ProgressSlot->SetHorizontalAlignment(HAlign_Center);
			ProgressSlot->SetPadding(FMargin(8.0f, 2.0f, 8.0f, 2.0f));
		}
	}

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	if (StatusText)
	{
		StatusText->SetJustification(ETextJustify::Center);
		StatusText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		StatusText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));

		FSlateFontInfo StatusFont = StatusText->GetFont();
		StatusFont.Size = 14;
		StatusText->SetFont(StatusFont);

		if (UVerticalBoxSlot* StatusSlot = RootBox->AddChildToVerticalBox(StatusText))
		{
			StatusSlot->SetHorizontalAlignment(HAlign_Center);
			StatusSlot->SetPadding(FMargin(8.0f, 2.0f, 8.0f, 4.0f));
		}
	}
}

void UBlackoutReviveProgressWidget::ResolveOptionalWidgetsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!ProgressBar)
	{
		ProgressBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("ProgressBar")));
	}

	if (!StatusText)
	{
		StatusText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("StatusText")));
	}
}

void UBlackoutReviveProgressWidget::SetInteractionProgressData(const FBlackoutInteractionPromptData& InInteractionPromptData)
{
	InteractionPromptData = InInteractionPromptData;
	ResolveOptionalWidgetsFromTree();

	const bool bShowProgressWidget =
		InteractionPromptData.bIsVisible &&
		InteractionPromptData.State == EBlackoutInteractionPromptState::InProgress;
	const bool bShowStatus = bShowProgressWidget && !InteractionPromptData.StatusText.IsEmpty();
	const bool bShowProgress = bShowProgressWidget && InteractionPromptData.bShowProgress;
	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;

	SetVisibility(bShowProgressWidget ? VisibleState : HiddenState);

	if (ProgressBar)
	{
		ProgressBar->SetPercent(InteractionPromptData.ProgressNormalized);
		ProgressBar->SetVisibility(bShowProgress ? VisibleState : HiddenState);
	}

	if (StatusText)
	{
		StatusText->SetText(InteractionPromptData.StatusText);
		StatusText->SetColorAndOpacity(FSlateColor(DefaultTextColor));
		StatusText->SetVisibility(bShowStatus ? VisibleState : HiddenState);
	}

	ReceiveInteractionProgressChanged(InteractionPromptData);
}
