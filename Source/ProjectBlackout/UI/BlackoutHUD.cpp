#include "UI/BlackoutHUD.h"

#include "UI/BlackoutEnemyHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/PlayerController.h"
#include "UI/BlackoutHUDWidget.h"
#include "UI/BlackoutHUDWidgetController.h"
#include "UI/BlackoutEnemyHUDWidgetController.h"
#include "UI/BlackoutPartyRosterWidgetController.h"

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

bool ABlackoutHUD::ShowDamageNumberAtWorldLocation(
	float DamageAmount,
	const FVector& WorldLocation,
	bool bIsCritical)
{
	if (!HUDWidget)
	{
		return false;
	}

	return HUDWidget->ShowDamageNumberAtWorldLocation(DamageAmount, WorldLocation, bIsCritical);
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
	
	if (EnemyHUDWidgetClass)
	{
		EnemyHUDWidget = CreateWidget<UBlackoutEnemyHUDWidget>(PlayerOwner, EnemyHUDWidgetClass);
		if (EnemyHUDWidget)
		{
			EnemyHUDWidget->AddToViewport();
		}
	}
}

void ABlackoutHUD::CreateWidgetController()
{
	if (!HUDWidget)
	{
		BO_LOG_CORE(Error, "WidgetController 생성 실패: HUDWidget이 유효하지 않습니다.");
		return;
	}
	
	if (!EnemyHUDWidgetController)
	{
		EnemyHUDWidgetController = NewObject<UBlackoutEnemyHUDWidgetController>(this);
		if (EnemyHUDWidget)
		{
			EnemyHUDWidget->SetWidgetController(EnemyHUDWidgetController);
		}
	}

	const bool bCreatedController = !HUDWidgetController;
	if (bCreatedController)
	{
		HUDWidgetController = NewObject<UBlackoutHUDWidgetController>(this);
		if (!HUDWidgetController)
		{
			BO_LOG_CORE(Error, "WidgetController 생성에 실패했습니다.");
			return;
		}
	}

	if (!HUDWidgetController->Initialize(PlayerOwner))
	{
		if (bCreatedController)
		{
			HUDWidgetController = nullptr;
		}
		return;
	}

	if (HUDWidget->GetWidgetController() != HUDWidgetController)
	{
		HUDWidget->SetWidgetController(HUDWidgetController);
	}

	HUDWidgetController->BroadcastInitialValues();

	if (!PartyRosterWidgetController)
	{
		PartyRosterWidgetController = NewObject<UBlackoutPartyRosterWidgetController>(this);
	}

	if (PartyRosterWidgetController && PartyRosterWidgetController->Initialize(PlayerOwner))
	{
		HUDWidget->SetPartyRosterWidgetController(PartyRosterWidgetController);
		PartyRosterWidgetController->BroadcastInitialRoster();
	}
}
