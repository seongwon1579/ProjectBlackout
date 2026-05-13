#pragma once


#include  "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * ClientTravel Loading 스크린
 * 
 */
class SBlackoutLoadingScreen : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBlackoutLoadingScreen){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
};