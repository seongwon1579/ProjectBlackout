#include "UI/BlackoutHUDWidget.h"

#include "Core/BlackoutLog.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "UI/BlackoutConsumableSlotsWidget.h"
#include "UI/BlackoutHUDWidgetController.h"
#include "UI/BlackoutValueBarWidget.h"
#include "UI/BlackoutWeaponAmmoWidget.h"

void UBlackoutHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UBlackoutHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FBlackoutImpactIndicatorData ImpactIndicatorData;
	if (WidgetController)
	{
		WidgetController->GetImpactIndicatorData(ImpactIndicatorData);
	}

	UpdateImpactIndicator(ImpactIndicatorData);
	ReceiveSpreadUpdated(ImpactIndicatorData.SpreadNormalized);
}

void UBlackoutHUDWidget::NativeDestruct()
{
	UnbindWidgetControllerCallbacks();

	Super::NativeDestruct();
}

void UBlackoutHUDWidget::SetWidgetController(UBlackoutHUDWidgetController* InWidgetController)
{
	if (!InWidgetController)
	{
		BO_LOG_CORE(Warning, "WidgetController가 유효하지 않아 HUD 위젯에 연결하지 않았습니다.");
		return;
	}

	UnbindWidgetControllerCallbacks();
	WidgetController = InWidgetController;

	WidgetController->OnHealthChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleHealthChanged);
	WidgetController->OnStaminaChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleStaminaChanged);
	WidgetController->OnAmmoChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleAmmoChanged);
	WidgetController->OnEquippedWeaponChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleEquippedWeaponChanged);
	WidgetController->OnAimingChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleAimingChanged);
	WidgetController->OnWeaponAmmoDisplayChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleWeaponAmmoDisplayChanged);
	WidgetController->OnConsumablesChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumablesChanged);
	WidgetController->OnConsumableSlotsChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumableSlotsChanged);

	ReceiveWidgetControllerSet();
}

void UBlackoutHUDWidget::UnbindWidgetControllerCallbacks()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnHealthChanged.RemoveAll(this);
	WidgetController->OnStaminaChanged.RemoveAll(this);
	WidgetController->OnAmmoChanged.RemoveAll(this);
	WidgetController->OnEquippedWeaponChanged.RemoveAll(this);
	WidgetController->OnAimingChanged.RemoveAll(this);
	WidgetController->OnWeaponAmmoDisplayChanged.RemoveAll(this);
	WidgetController->OnConsumablesChanged.RemoveAll(this);
	WidgetController->OnConsumableSlotsChanged.RemoveAll(this);
}

void UBlackoutHUDWidget::UpdateImpactIndicator(const FBlackoutImpactIndicatorData& ImpactIndicatorData) const
{
	if (!ImpactIndicatorWidget)
	{
		return;
	}

	ImpactIndicatorWidget->SetVisibility(ImpactIndicatorData.bIsVisible
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Hidden);

	if (!ImpactIndicatorData.bIsVisible)
	{
		ImpactIndicatorWidget->SetRenderScale(FVector2D::UnitVector);
		return;
	}

	FLinearColor IndicatorColor = ImpactIndicatorDefaultColor;
	if (ImpactIndicatorData.bIsOccludedFromCamera)
	{
		IndicatorColor = ImpactIndicatorOccludedColor;
	}
	else if (ImpactIndicatorData.bTargetMismatch)
	{
		IndicatorColor = ImpactIndicatorMismatchColor;
	}

	ApplyImpactIndicatorColor(IndicatorColor);
	ImpactIndicatorWidget->SetRenderOpacity(1.0f);

	const float SpreadScale = FMath::Lerp(1.0f, MaxSpreadIndicatorScale, ImpactIndicatorData.SpreadNormalized);
	ImpactIndicatorWidget->SetRenderScale(FVector2D(SpreadScale, SpreadScale));

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ImpactIndicatorWidget->Slot))
	{
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(ImpactIndicatorData.ScreenPosition);
		return;
	}

	ImpactIndicatorWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	ImpactIndicatorWidget->SetRenderTranslation(ImpactIndicatorData.ScreenPosition);
}

void UBlackoutHUDWidget::ApplyImpactIndicatorColor(const FLinearColor& IndicatorColor) const
{
	if (UImage* IndicatorImage = Cast<UImage>(ImpactIndicatorWidget))
	{
		IndicatorImage->SetColorAndOpacity(IndicatorColor);
		return;
	}

	if (UBorder* IndicatorBorder = Cast<UBorder>(ImpactIndicatorWidget))
	{
		IndicatorBorder->SetBrushColor(IndicatorColor);
	}
}

void UBlackoutHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetValue(CurrentHealth, MaxHealth);
	}

	ReceiveHealthChanged(CurrentHealth, MaxHealth);
}

void UBlackoutHUDWidget::HandleStaminaChanged(float CurrentStamina, float MaxStamina)
{
	if (StaminaBarWidget)
	{
		StaminaBarWidget->SetValue(CurrentStamina, MaxStamina);
	}

	ReceiveStaminaChanged(CurrentStamina, MaxStamina);
}

void UBlackoutHUDWidget::HandleAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo)
{
	ReceiveAmmoChanged(ClipAmmo, MaxClipAmmo, ReserveAmmo);
}

void UBlackoutHUDWidget::HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag)
{
	ReceiveEquippedWeaponChanged(EquippedWeapon, WeaponSlotTag);
}

void UBlackoutHUDWidget::HandleAimingChanged(bool bIsAiming, int32 CrosshairType)
{
	ReceiveAimingChanged(bIsAiming, CrosshairType);
}

void UBlackoutHUDWidget::HandleWeaponAmmoDisplayChanged(
	const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
	const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
	bool bPlaySwapAnimation)
{
	if (AmmoWidget)
	{
		AmmoWidget->SetWeaponAmmoData(PrimaryWeaponData, SecondaryWeaponData, bPlaySwapAnimation);
	}

	ReceiveWeaponAmmoDisplayChanged(PrimaryWeaponData, SecondaryWeaponData, bPlaySwapAnimation);
}

void UBlackoutHUDWidget::HandleConsumablesChanged(int32 BloodRootCount, int32 GulSerumCount)
{
	ReceiveConsumablesChanged(BloodRootCount, GulSerumCount);
}

void UBlackoutHUDWidget::HandleConsumableSlotsChanged(
	const FBlackoutConsumableSlotData& BloodRootData,
	const FBlackoutConsumableSlotData& GulSerumData)
{
	if (ConsumableSlotsWidget)
	{
		ConsumableSlotsWidget->SetConsumableSlotData(BloodRootData, GulSerumData);
	}

	ReceiveConsumableSlotsChanged(BloodRootData, GulSerumData);
}
