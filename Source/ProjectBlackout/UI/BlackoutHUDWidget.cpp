#include "UI/BlackoutHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/BlackoutLog.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UI/BlackoutDamageNumberWidget.h"
#include "UI/BlackoutConsumableSlotsWidget.h"
#include "UI/BlackoutDownedStateWidget.h"
#include "UI/BlackoutHUDWidgetController.h"
#include "UI/BlackoutInteractionPromptWidget.h"
#include "UI/BlackoutPartyRosterWidget.h"
#include "UI/BlackoutPartyRosterWidgetController.h"
#include "UI/BlackoutRelicWidget.h"
#include "UI/BlackoutReviveProgressWidget.h"
#include "UI/BlackoutSpectatorWidget.h"
#include "UI/BlackoutValueBarWidget.h"
#include "UI/BlackoutWeaponAmmoWidget.h"
#include "UI/BlackoutSurrenderVoteWidget.h"

namespace
{
	const TCHAR* DefaultRevivePromptWidgetClassPath =
		TEXT("/Game/_BP/UI/WBP/WBP_RevivePrompt.WBP_RevivePrompt_C");
	const TCHAR* DefaultReviveProgressWidgetClassPath =
		TEXT("/Game/_BP/UI/WBP/WBP_ReviveProgress.WBP_ReviveProgress_C");
}

void UBlackoutHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ResolveRevivePromptBindingsFromTree();
	ResolveReviveProgressBindingsFromTree();
	EnsureRevivePromptWidget();
	EnsureReviveProgressWidget();
}

void UBlackoutHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveRevivePromptBindingsFromTree();
	ResolveReviveProgressBindingsFromTree();
	EnsureRevivePromptWidget();
	EnsureReviveProgressWidget();
}

void UBlackoutHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ResolveRevivePromptBindingsFromTree();
	ResolveReviveProgressBindingsFromTree();

	if (!RevivePromptWidget && !RevivePromptContainer && !RevivePromptTextWidget && !ReviveStatusTextWidget && !ReviveProgressBarWidget)
	{
		EnsureRevivePromptWidget();
	}

	if (!ReviveProgressWidget)
	{
		EnsureReviveProgressWidget();
	}

	FBlackoutImpactIndicatorData ImpactIndicatorData;
	FBlackoutInteractionPromptData InteractionPromptData;
	FBlackoutDownedStateHUDData DownedStateHUDData;
	if (WidgetController)
	{
		WidgetController->GetImpactIndicatorData(ImpactIndicatorData);
		WidgetController->GetInteractionPromptData(InteractionPromptData);
		WidgetController->GetDownedStateHUDData(DownedStateHUDData);
	}

	UpdateImpactIndicator(ImpactIndicatorData);
	UpdateInteractionPrompt(InteractionPromptData);
	ReceiveSpreadUpdated(ImpactIndicatorData.SpreadNormalized);

	// 사망/부활 잔여 시간은 매 틱 부드럽게 갱신해야 하므로 폴링 결과를 그대로 다운 상태 위젯에 전달합니다.
	if (DownedStateWidget && DownedStateHUDData.HUDMode != EBlackoutHUDMode::Combat)
	{
		DownedStateWidget->SetDownedStateHUDData(DownedStateHUDData);
	}

	// 관전 모드에서는 ViewTarget이 사망/부활/관전 순환 등으로 자주 바뀌므로 닉네임을 매 틱 갱신합니다.
	if (SpectatorWidget && WidgetController && WidgetController->GetCurrentHUDMode() == EBlackoutHUDMode::Spectator)
	{
		FText SpectatorTargetName;
		WidgetController->GetSpectatorTargetName(SpectatorTargetName);
		SpectatorWidget->SetSpectatorTargetName(SpectatorTargetName);
	}
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

	if (StaminaBarWidget)
	{
		StaminaBarWidget->SetUseInterpolation(true);
	}

	WidgetController->OnHealthChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleHealthChanged);
	WidgetController->OnStaminaChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleStaminaChanged);
	WidgetController->OnAmmoChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleAmmoChanged);
	WidgetController->OnEquippedWeaponChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleEquippedWeaponChanged);
	WidgetController->OnAimingChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleAimingChanged);
	WidgetController->OnWeaponAmmoDisplayChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleWeaponAmmoDisplayChanged);
	WidgetController->OnRelicChargesChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleRelicChargesChanged);
	WidgetController->OnConsumablesChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumablesChanged);
	WidgetController->OnConsumableSlotsChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleConsumableSlotsChanged);
	WidgetController->OnHUDModeChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleHUDModeChanged);
	WidgetController->OnDownedStateHUDDataChanged.AddDynamic(this, &UBlackoutHUDWidget::HandleDownedStateHUDDataChanged);

	// 초기 모드로 가시성을 한 번 정렬해 첫 프레임에 부적절한 위젯이 노출되지 않도록 합니다.
	ApplyHUDMode(WidgetController->GetCurrentHUDMode());

	if (SurrenderVoteWidget)
	{
		SurrenderVoteWidget->SetWidgetController(InWidgetController);
	}

	ReceiveWidgetControllerSet();
}

void UBlackoutHUDWidget::SetPartyRosterWidgetController(
	UBlackoutPartyRosterWidgetController* InPartyRosterWidgetController)
{
	if (!InPartyRosterWidgetController)
	{
		BO_LOG_CORE(Warning, "파티 로스터 컨트롤러가 유효하지 않아 HUD 위젯에 연결하지 않았습니다.");
		return;
	}

	PartyRosterWidgetController = InPartyRosterWidgetController;

	if (PartyRosterWidget)
	{
		PartyRosterWidget->SetWidgetController(PartyRosterWidgetController);
	}
	else
	{
		BO_LOG_CORE(Verbose, "PartyRosterWidget이 바인딩되지 않아 파티 상태 패널 위젯 연결을 건너뜁니다.");
	}

	ReceivePartyRosterControllerSet(PartyRosterWidgetController);
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
	WidgetController->OnHUDModeChanged.RemoveAll(this);
	WidgetController->OnDownedStateHUDDataChanged.RemoveAll(this);
}

void UBlackoutHUDWidget::EnsureRevivePromptWidget()
{
	ResolveRevivePromptBindingsFromTree();

	if (RevivePromptWidget)
	{
		// 블루프린트에 직접 배치된 부활 프롬프트 위젯이 있으면 그것을 우선 사용합니다.
		return;
	}

	UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
	if (!RootPanel)
	{
		BO_LOG_CORE(Warning, "HUD 루트가 PanelWidget이 아니어서 부활 프롬프트 위젯을 자동 배치하지 못했습니다.");
		return;
	}

	TSubclassOf<UBlackoutInteractionPromptWidget> PromptWidgetClass =
		LoadClass<UBlackoutInteractionPromptWidget>(nullptr, DefaultRevivePromptWidgetClassPath);
	if (!PromptWidgetClass)
	{
		BO_LOG_CORE(Error, "WBP_RevivePrompt 클래스를 찾지 못했습니다. 부활 프롬프트는 WBP 경로로만 생성됩니다.");
		return;
	}

	UBlackoutInteractionPromptWidget* CreatedRevivePromptWidget = CreateWidget<UBlackoutInteractionPromptWidget>(
		GetOwningPlayer(),
		PromptWidgetClass);
	if (!CreatedRevivePromptWidget)
	{
		BO_LOG_CORE(Error, "부활 프롬프트 위젯 인스턴스를 생성하지 못했습니다.");
		return;
	}

	RevivePromptWidget = CreatedRevivePromptWidget;
	RevivePromptWidget->SetVisibility(ESlateVisibility::Hidden);

	if (UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(RootPanel))
	{
		if (UCanvasPanelSlot* CanvasSlot = CanvasRoot->AddChildToCanvas(RevivePromptWidget))
		{
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.78f, 0.5f, 0.78f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(FVector2D::ZeroVector);
		}
		return;
	}

	RootPanel->AddChild(RevivePromptWidget);
}

void UBlackoutHUDWidget::EnsureReviveProgressWidget()
{
	ResolveReviveProgressBindingsFromTree();

	if (ReviveProgressWidget)
	{
		// 블루프린트에 직접 배치된 진행 UI가 있으면 그것을 우선 사용합니다.
		return;
	}

	UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
	if (!RootPanel)
	{
		BO_LOG_CORE(Warning, "HUD 루트가 PanelWidget이 아니어서 부활 진행 UI를 자동 배치하지 못했습니다.");
		return;
	}

	TSubclassOf<UBlackoutReviveProgressWidget> ProgressWidgetClass =
		LoadClass<UBlackoutReviveProgressWidget>(nullptr, DefaultReviveProgressWidgetClassPath);
	if (!ProgressWidgetClass)
	{
		// 진행 UI WBP가 아직 없으면 C++ 기본 위젯으로라도 즉시 동작하도록 합니다.
		ProgressWidgetClass = UBlackoutReviveProgressWidget::StaticClass();
	}

	UBlackoutReviveProgressWidget* CreatedReviveProgressWidget = CreateWidget<UBlackoutReviveProgressWidget>(
		GetOwningPlayer(),
		ProgressWidgetClass);
	if (!CreatedReviveProgressWidget)
	{
		BO_LOG_CORE(Error, "부활 진행 UI 위젯 인스턴스를 생성하지 못했습니다.");
		return;
	}

	ReviveProgressWidget = CreatedReviveProgressWidget;
	ReviveProgressWidget->SetVisibility(ESlateVisibility::Hidden);

	if (UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(RootPanel))
	{
		if (UCanvasPanelSlot* CanvasSlot = CanvasRoot->AddChildToCanvas(ReviveProgressWidget))
		{
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetAnchors(ReviveProgressScreenAnchors);
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(ReviveProgressScreenOffset);
		}
		return;
	}

	RootPanel->AddChild(ReviveProgressWidget);
}

void UBlackoutHUDWidget::ResolveRevivePromptBindingsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!RevivePromptWidget)
	{
		RevivePromptWidget = Cast<UBlackoutInteractionPromptWidget>(WidgetTree->FindWidget(TEXT("RevivePromptWidget")));
	}

	if (!RevivePromptContainer)
	{
		RevivePromptContainer = WidgetTree->FindWidget(TEXT("RevivePromptContainer"));
	}

	if (!RevivePromptTextWidget)
	{
		RevivePromptTextWidget = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("RevivePromptTextWidget")));
	}

	if (!ReviveStatusTextWidget)
	{
		ReviveStatusTextWidget = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ReviveStatusTextWidget")));
	}

	if (!ReviveProgressBarWidget)
	{
		ReviveProgressBarWidget = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("ReviveProgressBarWidget")));
	}

	if (RevivePromptWidget)
	{
		return;
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* CandidateWidget : AllWidgets)
	{
		if (UBlackoutInteractionPromptWidget* CandidateRevivePromptWidget = Cast<UBlackoutInteractionPromptWidget>(CandidateWidget))
		{
			RevivePromptWidget = CandidateRevivePromptWidget;
			break;
		}
	}
}

void UBlackoutHUDWidget::ResolveReviveProgressBindingsFromTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!ReviveProgressWidget)
	{
		ReviveProgressWidget = Cast<UBlackoutReviveProgressWidget>(WidgetTree->FindWidget(TEXT("ReviveProgressWidget")));
	}

	if (ReviveProgressWidget)
	{
		return;
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* CandidateWidget : AllWidgets)
	{
		if (UBlackoutReviveProgressWidget* CandidateReviveProgressWidget = Cast<UBlackoutReviveProgressWidget>(CandidateWidget))
		{
			ReviveProgressWidget = CandidateReviveProgressWidget;
			break;
		}
	}
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

void UBlackoutHUDWidget::UpdateInteractionPrompt(const FBlackoutInteractionPromptData& InteractionPromptData)
{
	ResolveRevivePromptBindingsFromTree();
	ResolveReviveProgressBindingsFromTree();

	const bool bShowScreenProgress =
		InteractionPromptData.bIsVisible &&
		InteractionPromptData.State == EBlackoutInteractionPromptState::InProgress;
	const bool bShowWorldPrompt = InteractionPromptData.bIsVisible && !bShowScreenProgress;

	FBlackoutInteractionPromptData HiddenPromptData = InteractionPromptData;
	HiddenPromptData.bIsVisible = false;
	HiddenPromptData.bShowProgress = false;
	HiddenPromptData.ProgressNormalized = 0.0f;
	HiddenPromptData.State = EBlackoutInteractionPromptState::Hidden;
	HiddenPromptData.PromptText = FText::GetEmpty();
	HiddenPromptData.StatusText = FText::GetEmpty();

	if (bShowWorldPrompt &&
		!RevivePromptWidget &&
		!RevivePromptContainer &&
		!RevivePromptTextWidget &&
		!ReviveStatusTextWidget &&
		!ReviveProgressBarWidget)
	{
		EnsureRevivePromptWidget();
	}

	if (bShowScreenProgress && !ReviveProgressWidget)
	{
		EnsureReviveProgressWidget();
	}

	if (RevivePromptWidget)
	{
		RevivePromptWidget->SetInteractionPromptData(bShowWorldPrompt ? InteractionPromptData : HiddenPromptData);
	}

	if (ReviveProgressWidget)
	{
		ReviveProgressWidget->SetInteractionProgressData(bShowScreenProgress ? InteractionPromptData : HiddenPromptData);
	}

	const ESlateVisibility VisibleState = ESlateVisibility::HitTestInvisible;
	const ESlateVisibility HiddenState = ESlateVisibility::Hidden;
	const bool bShowPrompt = bShowWorldPrompt;
	const bool bShowStatus = bShowPrompt && !InteractionPromptData.StatusText.IsEmpty();
	const bool bShowProgress = bShowPrompt && InteractionPromptData.bShowProgress;

	auto ApplyTrackedPromptPosition = [this, &InteractionPromptData](UWidget* TargetWidget)
	{
		if (!TargetWidget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TargetWidget->Slot))
		{
			// 화면에 투영된 다운드 대상 머리 위 좌표에 위젯의 하단 중앙이 오도록 정렬합니다.
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
			CanvasSlot->SetPosition(InteractionPromptData.ScreenPosition + RevivePromptScreenOffset);
		}
	};

	auto ApplyScreenFixedProgressPosition = [this](UWidget* TargetWidget)
	{
		if (!TargetWidget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TargetWidget->Slot))
		{
			// 실제 소생 진행 UI는 살려주는 플레이어 화면 기준으로만 고정합니다.
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetAnchors(ReviveProgressScreenAnchors);
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(ReviveProgressScreenOffset);
		}
	};

	if (RevivePromptContainer)
	{
		RevivePromptContainer->SetVisibility(bShowPrompt ? VisibleState : HiddenState);
	}

	if (RevivePromptTextWidget)
	{
		RevivePromptTextWidget->SetText(InteractionPromptData.PromptText);
		RevivePromptTextWidget->SetColorAndOpacity(FSlateColor(RevivePromptDefaultColor));
		RevivePromptTextWidget->SetVisibility(bShowPrompt ? VisibleState : HiddenState);
	}

	if (ReviveStatusTextWidget)
	{
		ReviveStatusTextWidget->SetText(InteractionPromptData.StatusText);
		ReviveStatusTextWidget->SetColorAndOpacity(FSlateColor(
			InteractionPromptData.bIsStatusError ? RevivePromptErrorColor : RevivePromptDefaultColor));
		ReviveStatusTextWidget->SetVisibility(bShowStatus ? VisibleState : HiddenState);
	}

	if (ReviveProgressBarWidget)
	{
		ReviveProgressBarWidget->SetPercent(InteractionPromptData.ProgressNormalized);
		ReviveProgressBarWidget->SetVisibility(bShowProgress ? VisibleState : HiddenState);
	}

	if (bShowPrompt)
	{
		if (RevivePromptWidget)
		{
			ApplyTrackedPromptPosition(RevivePromptWidget);
		}
		else if (RevivePromptContainer)
		{
			ApplyTrackedPromptPosition(RevivePromptContainer);
		}
	}

	if (bShowScreenProgress && ReviveProgressWidget)
	{
		ApplyScreenFixedProgressPosition(ReviveProgressWidget);
	}

	ReceiveInteractionPromptUpdated(InteractionPromptData);
	ReceiveRevivePromptUpdated(InteractionPromptData);
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

void UBlackoutHUDWidget::ApplyHUDMode(EBlackoutHUDMode InHUDMode)
{
	// 다운/관전 상태에서는 기본 전투 HUD 레이어를 한 번에 숨기고, 다운 모드일 때만 다운 위젯을 노출합니다.
	const bool bShowBasicCombatHUD = InHUDMode == EBlackoutHUDMode::Combat;
	const bool bShowDownedWidget =
		InHUDMode == EBlackoutHUDMode::DownedDeathTimer ||
		InHUDMode == EBlackoutHUDMode::DownedReviveTimer;
	const bool bShowSpectatorWidget = InHUDMode == EBlackoutHUDMode::Spectator;

	if (BasicCombatHUDLayer)
	{
		BasicCombatHUDLayer->SetVisibility(bShowBasicCombatHUD
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	}

	if (DownedStateWidget)
	{
		DownedStateWidget->SetVisibility(bShowDownedWidget
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Hidden);
	}

	if (SpectatorWidget)
	{
		SpectatorWidget->SetVisibility(bShowSpectatorWidget
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Hidden);
	}

	ReceiveHUDModeChanged(InHUDMode);
}

void UBlackoutHUDWidget::HandleHUDModeChanged(EBlackoutHUDMode HUDMode)
{
	ApplyHUDMode(HUDMode);
}

void UBlackoutHUDWidget::HandleDownedStateHUDDataChanged(const FBlackoutDownedStateHUDData& DownedStateHUDData)
{
	if (DownedStateWidget)
	{
		DownedStateWidget->SetDownedStateHUDData(DownedStateHUDData);
	}

	ReceiveDownedStateHUDDataChanged(DownedStateHUDData);
}
