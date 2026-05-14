#include "UI/BlackoutHUDWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/BlackoutLog.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UI/BlackoutDamageNumberWidget.h"
#include "UI/BlackoutConsumableSlotsWidget.h"
#include "UI/BlackoutHUDWidgetController.h"
#include "UI/BlackoutRelicWidget.h"
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

int32 UBlackoutHUDWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 TrajectoryLayerId = PaintProjectileTrajectoryDots(AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);

	return Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		TrajectoryLayerId + 1,
		InWidgetStyle,
		bParentEnabled);
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
	WidgetController->OnRelicChargesChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleRelicChargesChanged);
	WidgetController->OnConsumablesChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumablesChanged);
	WidgetController->OnConsumableSlotsChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumableSlotsChanged);

	ReceiveWidgetControllerSet();
}

bool UBlackoutHUDWidget::ShowDamageNumberAtWorldLocation(
	float DamageAmount,
	const FVector& WorldLocation,
	bool bIsCritical)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (!OwningPlayerController || !OwningPlayerController->IsLocalController())
	{
		return false;
	}

	if (CNV_DamageNumbers && DamageNumberWidgetClass)
	{
		UBlackoutDamageNumberWidget* DamageNumberWidget =
			CreateWidget<UBlackoutDamageNumberWidget>(OwningPlayerController, DamageNumberWidgetClass);
		if (DamageNumberWidget)
		{
			if (UCanvasPanelSlot* CanvasPanelSlot = CNV_DamageNumbers->AddChildToCanvas(DamageNumberWidget))
			{
				const FVector2D RandomOffset(
					FMath::FRandRange(-DamageNumberRandomOffsetX, DamageNumberRandomOffsetX),
					FMath::FRandRange(-DamageNumberRandomOffsetY, DamageNumberRandomOffsetY));

				DamageNumberWidget->SetCanvasSlot(CanvasPanelSlot);
				DamageNumberWidget->InitializeDamageNumber(
					DamageAmount,
					bIsCritical,
					WorldLocation,
					RandomOffset);
				return true;
			}
		}
	}

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		OwningPlayerController,
		WorldLocation,
		ScreenPosition,
		true))
	{
		return false;
	}

	// 실제 숫자 WBP 생성은 블루프린트에서 처리하고, C++은 좌표 전달만 담당합니다.
	ReceiveDamageNumberRequested(DamageAmount, ScreenPosition, bIsCritical);
	return true;
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
	WidgetController->OnRelicChargesChanged.RemoveAll(this);
	WidgetController->OnConsumablesChanged.RemoveAll(this);
	WidgetController->OnConsumableSlotsChanged.RemoveAll(this);
}

void UBlackoutHUDWidget::UpdateImpactIndicator(const FBlackoutImpactIndicatorData& ImpactIndicatorData)
{
	CachedTrajectoryPoints = ImpactIndicatorData.bIsVisible
		? ImpactIndicatorData.TrajectoryPoints
		: TArray<FBlackoutTrajectoryPointData>();

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
	if (ImpactIndicatorData.bProjectileImpactFuseInactive)
	{
		IndicatorColor = ImpactIndicatorMismatchColor;
	}
	else if (ImpactIndicatorData.bIsOccludedFromCamera)
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

int32 UBlackoutHUDWidget::PaintProjectileTrajectoryDots(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle) const
{
	if (CachedTrajectoryPoints.Num() <= 2)
	{
		return LayerId;
	}

	// 기본 흰색 브러시에 포인트 상태별 색상을 곱해 유탄 궤적 점을 그립니다.
	const FVector2D DotSize(TrajectoryDotSize, TrajectoryDotSize);
	const FSlateBrush* DotBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	// 총구와 True Hit 인디케이터를 가리지 않도록 앞쪽 첫 점과 끝쪽 일부 점은 렌더링하지 않습니다.
	int32 CurrentLayerId = LayerId;
	for (int32 PointIndex = 1; PointIndex < CachedTrajectoryPoints.Num() - 3; ++PointIndex)
	{
		const FBlackoutTrajectoryPointData& TrajectoryPoint = CachedTrajectoryPoints[PointIndex];
		const FVector2D DotPosition = TrajectoryPoint.ScreenPosition - DotSize * 0.5f;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			CurrentLayerId,
			AllottedGeometry.ToPaintGeometry(DotSize, FSlateLayoutTransform(DotPosition)),
			DotBrush,
			ESlateDrawEffect::None,
			InWidgetStyle.GetColorAndOpacityTint() * ResolveTrajectoryColor(TrajectoryPoint.VisualState));
		++CurrentLayerId;
	}

	return CurrentLayerId;
}

FLinearColor UBlackoutHUDWidget::ResolveTrajectoryColor(EBlackoutTrajectoryVisualState VisualState) const
{
	switch (VisualState)
	{
	case EBlackoutTrajectoryVisualState::FuseInactive:
		return TrajectoryFuseInactiveColor;
	case EBlackoutTrajectoryVisualState::Occluded:
		return TrajectoryOccludedColor;
	case EBlackoutTrajectoryVisualState::Normal:
	default:
		return TrajectoryNormalColor;
	}
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

void UBlackoutHUDWidget::HandleRelicChargesChanged(int32 CurrentCharges, int32 MaxCharges)
{
	if (RelicWidget)
	{
		RelicWidget->SetRelicCharges(CurrentCharges, MaxCharges);
	}

	ReceiveRelicChargesChanged(CurrentCharges, MaxCharges);
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
