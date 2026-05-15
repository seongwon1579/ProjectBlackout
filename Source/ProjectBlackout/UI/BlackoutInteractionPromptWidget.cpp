#include "UI/BlackoutInteractionPromptWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBlackoutInteractionPromptWidget::NativePreConstruct()
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
	}

	SetInteractionPromptData(PreviewPromptData);
}

void UBlackoutInteractionPromptWidget::EnsureDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RevivePromptRoot"));
	if (!RootBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootBox;

	PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));
	if (PromptText)
	{
		PromptText->SetJustification(ETextJustify::Center);
		PromptText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		PromptText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));

		FSlateFontInfo PromptFont = PromptText->GetFont();
		PromptFont.Size = 20;
		PromptText->SetFont(PromptFont);

		if (UVerticalBoxSlot* PromptSlot = RootBox->AddChildToVerticalBox(PromptText))
		{
			PromptSlot->SetHorizontalAlignment(HAlign_Center);
			PromptSlot->SetPadding(FMargin(8.0f, 4.0f, 8.0f, 2.0f));
		}
	}

	USizeBox* ProgressBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ProgressBox"));
	ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
	if (ProgressBox && ProgressBar)
	{
		ProgressBox->SetWidthOverride(132.0f);
		ProgressBox->SetHeightOverride(10.0f);
		ProgressBar->SetFillColorAndOpacity(FLinearColor(0.08f, 0.9f, 0.5f, 1.0f));
		ProgressBar->SetPercent(0.0f);
		ProgressBox->AddChild(ProgressBar);

		if (UVerticalBoxSlot* ProgressSlot = RootBox->AddChildToVerticalBox(ProgressBox))
		{
			ProgressSlot->SetHorizontalAlignment(HAlign_Center);
			ProgressSlot->SetPadding(FMargin(8.0f, 0.0f, 8.0f, 2.0f));
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

void UBlackoutInteractionPromptWidget::ResolveOptionalWidgetsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!PromptText)
	{
		PromptText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("PromptText")));
	}

	if (!StatusText)
	{
		StatusText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("StatusText")));
	}

	if (!ProgressBar)
	{
		ProgressBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("ProgressBar")));
	}
}

void UBlackoutInteractionPromptWidget::SetInteractionPromptData(const FBlackoutInteractionPromptData& InInteractionPromptData)
{
	InteractionPromptData = InInteractionPromptData;
	ResolveOptionalWidgetsFromTree();

	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;
	const bool bShowPrompt = InteractionPromptData.bIsVisible;
	const bool bShowStatus = bShowPrompt && !InteractionPromptData.StatusText.IsEmpty();
	const bool bShowProgress = bShowPrompt && InteractionPromptData.bShowProgress;

	SetVisibility(bShowPrompt ? VisibleState : HiddenState);

	if (PromptText)
	{
		PromptText->SetText(InteractionPromptData.PromptText);
		PromptText->SetColorAndOpacity(FSlateColor(DefaultTextColor));
		PromptText->SetVisibility(bShowPrompt ? VisibleState : HiddenState);
	}

	if (StatusText)
	{
		StatusText->SetText(InteractionPromptData.StatusText);
		StatusText->SetColorAndOpacity(FSlateColor(
			InteractionPromptData.bIsStatusError ? ErrorTextColor : DefaultTextColor));
		StatusText->SetVisibility(bShowStatus ? VisibleState : HiddenState);
	}

	if (ProgressBar)
	{
		ProgressBar->SetPercent(InteractionPromptData.ProgressNormalized);
		ProgressBar->SetVisibility(bShowProgress ? VisibleState : HiddenState);
	}

	ReceiveInteractionPromptChanged(InteractionPromptData);
	ReceiveRevivePromptChanged(InteractionPromptData);
}
