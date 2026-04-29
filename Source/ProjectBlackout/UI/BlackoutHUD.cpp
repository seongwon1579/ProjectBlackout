#include "UI/BlackoutHUD.h"

#include "Blueprint/UserWidget.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/PlayerController.h"
#include "UI/BlackoutHUDWidget.h"

void ABlackoutHUD::BeginPlay()
{
	Super::BeginPlay();

	InitHUD();
}

void ABlackoutHUD::InitHUD()
{
	if (!PlayerOwner)
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: PlayerOwner가 유효하지 않습니다.");
		return;
	}

	if (!PlayerOwner->IsLocalController())
	{
		return;
	}

	CreateHUDWidget();
}

void ABlackoutHUD::CreateHUDWidget()
{
	if (HUDWidget)
	{
		return;
	}

	if (!HUDWidgetClass)
	{
		BO_LOG_CORE(Warning, "HUDWidgetClass가 지정되지 않아 인게임 HUD를 생성하지 않았습니다.");
		return;
	}

	HUDWidget = CreateWidget<UBlackoutHUDWidget>(PlayerOwner, HUDWidgetClass);
	if (!HUDWidget)
	{
		BO_LOG_CORE(Error, "HUD 위젯 생성에 실패했습니다. HUDWidgetClass=%s", *GetNameSafe(HUDWidgetClass.Get()));
		return;
	}

	HUDWidget->AddToViewport();
}
