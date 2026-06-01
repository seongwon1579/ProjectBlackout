#include "UI/BlackoutSpectatorWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UBlackoutSpectatorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ResolveOptionalWidgetsFromTree();
}

void UBlackoutSpectatorWidget::SetSpectatorTargetName(const FText& InTargetName)
{
	CurrentTargetName = InTargetName;

	ResolveOptionalWidgetsFromTree();

	if (TargetNameText)
	{
		TargetNameText->SetText(CurrentTargetName);
		TargetNameText->SetColorAndOpacity(FSlateColor(TargetNameTextColor));
	}

	ReceiveSpectatorTargetNameChanged(CurrentTargetName);
}

void UBlackoutSpectatorWidget::ResolveOptionalWidgetsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!TargetNameText)
	{
		TargetNameText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TargetNameText")));
	}
}
