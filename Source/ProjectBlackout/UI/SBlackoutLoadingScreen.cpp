#include "SBlackoutLoadingScreen.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void SBlackoutLoadingScreen::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SImage).ColorAndOpacity(
				FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
		]

		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("LOADING...")))
			.
			Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			.ColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f))
			.Justification(ETextJustify::Center)
		]
	];
}
