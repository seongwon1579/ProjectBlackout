#include "UI/BlackoutDamageNumberWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"

void UBlackoutDamageNumberWidget::InitializeDamageNumber(
	float InDamageAmount,
	bool bInIsCritical,
	const FVector& InWorldLocation,
	const FVector2D& InRandomScreenOffset)
{
	DamageAmount = InDamageAmount;
	bIsCritical = bInIsCritical;
	TargetWorldLocation = InWorldLocation;
	RandomScreenOffset = InRandomScreenOffset;
	CachedPlayerController = GetOwningPlayer();

	ReceiveDamageNumberInitialized(DamageAmount, bIsCritical);
	RefreshProjectedScreenPosition();
}

void UBlackoutDamageNumberWidget::SetCanvasSlot(UCanvasPanelSlot* InCanvasSlot)
{
	CachedCanvasPanelSlot = InCanvasSlot;

	if (InCanvasSlot)
	{
		InCanvasSlot->SetAutoSize(true);
		InCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	}

	RefreshProjectedScreenPosition();
}

void UBlackoutDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshProjectedScreenPosition();
}

void UBlackoutDamageNumberWidget::NativeDestruct()
{
	CachedCanvasPanelSlot.Reset();
	CachedPlayerController.Reset();

	Super::NativeDestruct();
}

void UBlackoutDamageNumberWidget::RefreshProjectedScreenPosition() const
{
	APlayerController* PlayerController = CachedPlayerController.Get();
	UCanvasPanelSlot* CanvasPanelSlot = CachedCanvasPanelSlot.Get();
	if (!PlayerController || !CanvasPanelSlot || !PlayerController->IsLocalController())
	{
		return;
	}

	FVector2D ProjectedScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		PlayerController,
		TargetWorldLocation,
		ProjectedScreenPosition,
		true))
	{
		return;
	}

	CanvasPanelSlot->SetPosition(ProjectedScreenPosition + RandomScreenOffset);
}
