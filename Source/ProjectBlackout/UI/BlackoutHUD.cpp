#include "UI/BlackoutHUD.h"

#include "Blueprint/UserWidget.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/PlayerController.h"
#include "UI/BlackoutHUDWidget.h"
#include "UI/BlackoutHUDWidgetController.h"

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
	CreateWidgetController();
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

void ABlackoutHUD::CreateWidgetController()
{
	if (HUDWidgetController)
	{
		return;
	}

	if (!HUDWidget)
	{
		BO_LOG_CORE(Error, "WidgetController 생성 실패: HUDWidget이 유효하지 않습니다.");
		return;
	}

	HUDWidgetController = NewObject<UBlackoutHUDWidgetController>(this);
	if (!HUDWidgetController)
	{
		BO_LOG_CORE(Error, "WidgetController 생성에 실패했습니다.");
		return;
	}

	if (!HUDWidgetController->Initialize(PlayerOwner))
	{
		HUDWidgetController = nullptr;
		return;
	}

	HUDWidget->SetWidgetController(HUDWidgetController);
	HUDWidgetController->BroadcastInitialValues();
}
