#include "UI/BlackoutHUDWidget.h"

#include "Core/BlackoutLog.h"

void UBlackoutHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UBlackoutHUDWidget::SetWidgetController(UObject* InWidgetController)
{
	if (!InWidgetController)
	{
		BO_LOG_CORE(Warning, "WidgetController가 유효하지 않아 HUD 위젯에 연결하지 않았습니다.");
		return;
	}

	WidgetController = InWidgetController;
	ReceiveWidgetControllerSet();
}
