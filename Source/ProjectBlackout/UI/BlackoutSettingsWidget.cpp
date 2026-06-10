#include "UI/BlackoutSettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "InputCoreTypes.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/BlackoutGraphicsBlueprintLibrary.h"

namespace
{
	template <typename TWidgetType>
	void ResolveNamedWidget(UWidgetTree* WidgetTree, TObjectPtr<TWidgetType>& WidgetPointer, const FName WidgetName)
	{
		if (WidgetPointer || !WidgetTree)
		{
			return;
		}

		WidgetPointer = Cast<TWidgetType>(WidgetTree->FindWidget(WidgetName));
	}
}

void UBlackoutSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	ResolveOptionalBindings();
	PopulateStaticOptions();
	BindControlEvents();
	RefreshFromCurrentSettings();
	SelectSettingsTab(ActiveSettingsTab);
	SetKeyboardFocus();
}

void UBlackoutSettingsWidget::NativeDestruct()
{
	if (UpscalerComboBox)
	{
		UpscalerComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleUpscalerSelectionChanged);
	}

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleDLSSModeSelectionChanged);
	}

	if (FrameGenerationComboBox)
	{
		FrameGenerationComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleFrameGenerationSelectionChanged);
	}

	if (ReflexComboBox)
	{
		ReflexComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleReflexModeSelectionChanged);
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMasterVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleSFXVolumeChanged);
	}

	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMouseSensitivityChanged);
	}

	if (AimMouseSensitivitySlider)
	{
		AimMouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleAimMouseSensitivityChanged);
	}

	if (GraphicsTabButton)
	{
		GraphicsTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleGraphicsTabClicked);
	}

	if (AudioTabButton)
	{
		AudioTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleAudioTabClicked);
	}

	if (InputTabButton)
	{
		InputTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleInputTabClicked);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleResetClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleCloseClicked);
	}

	Super::NativeDestruct();
}

FReply UBlackoutSettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseSettingsWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UBlackoutSettingsWidget::RefreshFromCurrentSettings()
{
	const UBlackoutGraphicsUserSettings* GraphicsSettings = UBlackoutGraphicsBlueprintLibrary::GetBlackoutGraphicsUserSettings();
	if (!GraphicsSettings)
	{
		// 설정 객체를 찾지 못하더라도 UI가 기본 문구에 머무르지 않도록 기본값과 가용성 텍스트를 갱신합니다.
		ResetPendingSettingsToDefaults();
		SyncControlsFromPendingSettings();
		UpdateControlState();
		UpdateValueTexts();
		return;
	}

	PendingUpscalerMode = GraphicsSettings->GetUpscalerMode();
	PendingDLSSMode = GraphicsSettings->GetDLSSQualityMode();
	PendingFrameGenerationMode = GraphicsSettings->GetFrameGenerationMode();
	PendingReflexMode = GraphicsSettings->GetReflexModeOption();
	PendingMasterVolume = GraphicsSettings->GetMasterVolume();
	PendingMusicVolume = GraphicsSettings->GetMusicVolume();
	PendingSFXVolume = GraphicsSettings->GetSFXVolume();
	PendingMouseSensitivity = GraphicsSettings->GetMouseSensitivity();
	PendingAimMouseSensitivityMultiplier = GraphicsSettings->GetAimMouseSensitivityMultiplier();

	if (!UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationModeRuntimeAvailable(PendingFrameGenerationMode))
	{
		PendingFrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;
	}

	SyncControlsFromPendingSettings();
	UpdateControlState();
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::SyncControlsFromPendingSettings()
{
	if (UpscalerComboBox)
	{
		UpscalerComboBox->SetSelectedOption(GetUpscalerOptionLabel(PendingUpscalerMode));
	}

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->SetSelectedOption(GetDLSSModeOptionLabel(PendingDLSSMode));
	}

	if (FrameGenerationComboBox)
	{
		FrameGenerationComboBox->SetSelectedOption(GetFrameGenerationOptionLabel(PendingFrameGenerationMode));
	}

	if (ReflexComboBox)
	{
		ReflexComboBox->SetSelectedOption(GetReflexModeOptionLabel(PendingReflexMode));
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(PendingMasterVolume);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetValue(PendingMusicVolume);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetValue(PendingSFXVolume);
	}

	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue((PendingMouseSensitivity - 0.1f) / 2.9f);
	}

	if (AimMouseSensitivitySlider)
	{
		AimMouseSensitivitySlider->SetValue((PendingAimMouseSensitivityMultiplier - 0.1f) / 2.9f);
	}
}

void UBlackoutSettingsWidget::ResolveOptionalBindings()
{
	// WBP에서 Is Variable 체크가 빠졌더라도 이름이 일치하면 위젯을 찾아 연결합니다.
	ResolveNamedWidget(WidgetTree, UpscalerComboBox, TEXT("UpscalerComboBox"));
	ResolveNamedWidget(WidgetTree, DLSSModeComboBox, TEXT("DLSSModeComboBox"));
	ResolveNamedWidget(WidgetTree, FrameGenerationComboBox, TEXT("FrameGenerationComboBox"));
	ResolveNamedWidget(WidgetTree, ReflexComboBox, TEXT("ReflexComboBox"));
	ResolveNamedWidget(WidgetTree, MasterVolumeSlider, TEXT("MasterVolumeSlider"));
	ResolveNamedWidget(WidgetTree, MusicVolumeSlider, TEXT("MusicVolumeSlider"));
	ResolveNamedWidget(WidgetTree, SFXVolumeSlider, TEXT("SFXVolumeSlider"));
	ResolveNamedWidget(WidgetTree, MouseSensitivitySlider, TEXT("MouseSensitivitySlider"));
	ResolveNamedWidget(WidgetTree, AimMouseSensitivitySlider, TEXT("AimMouseSensitivitySlider"));
	ResolveNamedWidget(WidgetTree, MasterVolumeValueText, TEXT("MasterVolumeValueText"));
	ResolveNamedWidget(WidgetTree, MusicVolumeValueText, TEXT("MusicVolumeValueText"));
	ResolveNamedWidget(WidgetTree, SFXVolumeValueText, TEXT("SFXVolumeValueText"));
	ResolveNamedWidget(WidgetTree, MouseSensitivityValueText, TEXT("MouseSensitivityValueText"));
	ResolveNamedWidget(WidgetTree, AimMouseSensitivityValueText, TEXT("AimMouseSensitivityValueText"));
	ResolveNamedWidget(WidgetTree, RuntimeAvailabilityText, TEXT("RuntimeAvailabilityText"));
	ResolveNamedWidget(WidgetTree, GraphicsTabButton, TEXT("GraphicsTabButton"));
	ResolveNamedWidget(WidgetTree, AudioTabButton, TEXT("AudioTabButton"));
	ResolveNamedWidget(WidgetTree, InputTabButton, TEXT("InputTabButton"));
	ResolveNamedWidget(WidgetTree, SettingsContentSwitcher, TEXT("SettingsContentSwitcher"));
	ResolveNamedWidget(WidgetTree, ApplyButton, TEXT("ApplyButton"));
	ResolveNamedWidget(WidgetTree, ResetButton, TEXT("ResetButton"));
	ResolveNamedWidget(WidgetTree, CloseButton, TEXT("CloseButton"));
}

void UBlackoutSettingsWidget::PopulateStaticOptions()
{
	if (bOptionsPopulated)
	{
		return;
	}

	if (UpscalerComboBox)
	{
		UpscalerComboBox->ClearOptions();
		UpscalerComboBox->AddOption(GetUpscalerOptionLabel(EBlackoutUpscalerMode::TSR));
		UpscalerComboBox->AddOption(GetUpscalerOptionLabel(EBlackoutUpscalerMode::DLSS));
		UpscalerComboBox->AddOption(GetUpscalerOptionLabel(EBlackoutUpscalerMode::DLAA));
	}

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->ClearOptions();
		DLSSModeComboBox->AddOption(GetDLSSModeOptionLabel(EBlackoutDLSSQualityMode::Quality));
		DLSSModeComboBox->AddOption(GetDLSSModeOptionLabel(EBlackoutDLSSQualityMode::Balanced));
		DLSSModeComboBox->AddOption(GetDLSSModeOptionLabel(EBlackoutDLSSQualityMode::Performance));
		DLSSModeComboBox->AddOption(GetDLSSModeOptionLabel(EBlackoutDLSSQualityMode::UltraPerformance));
	}

	if (FrameGenerationComboBox)
	{
		FrameGenerationComboBox->ClearOptions();
		FrameGenerationComboBox->AddOption(GetFrameGenerationOptionLabel(EBlackoutFrameGenerationMode::Disabled));
		if (UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationModeRuntimeAvailable(EBlackoutFrameGenerationMode::On2X))
		{
			FrameGenerationComboBox->AddOption(GetFrameGenerationOptionLabel(EBlackoutFrameGenerationMode::On2X));
		}

		if (UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationModeRuntimeAvailable(EBlackoutFrameGenerationMode::On3X))
		{
			FrameGenerationComboBox->AddOption(GetFrameGenerationOptionLabel(EBlackoutFrameGenerationMode::On3X));
		}

		if (UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationModeRuntimeAvailable(EBlackoutFrameGenerationMode::On4X))
		{
			FrameGenerationComboBox->AddOption(GetFrameGenerationOptionLabel(EBlackoutFrameGenerationMode::On4X));
		}
	}

	if (ReflexComboBox)
	{
		ReflexComboBox->ClearOptions();
		ReflexComboBox->AddOption(GetReflexModeOptionLabel(EBlackoutReflexMode::Disabled));
		ReflexComboBox->AddOption(GetReflexModeOptionLabel(EBlackoutReflexMode::Enabled));
		ReflexComboBox->AddOption(GetReflexModeOptionLabel(EBlackoutReflexMode::EnabledPlusBoost));
	}

	bOptionsPopulated = true;
}

void UBlackoutSettingsWidget::BindControlEvents()
{
	if (UpscalerComboBox)
	{
		UpscalerComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleUpscalerSelectionChanged);
		UpscalerComboBox->OnSelectionChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleUpscalerSelectionChanged);
	}

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleDLSSModeSelectionChanged);
		DLSSModeComboBox->OnSelectionChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleDLSSModeSelectionChanged);
	}

	if (FrameGenerationComboBox)
	{
		FrameGenerationComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleFrameGenerationSelectionChanged);
		FrameGenerationComboBox->OnSelectionChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleFrameGenerationSelectionChanged);
	}

	if (ReflexComboBox)
	{
		ReflexComboBox->OnSelectionChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleReflexModeSelectionChanged);
		ReflexComboBox->OnSelectionChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleReflexModeSelectionChanged);
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMasterVolumeChanged);
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleMasterVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMusicVolumeChanged);
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleSFXVolumeChanged);
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleSFXVolumeChanged);
	}

	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleMouseSensitivityChanged);
		MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleMouseSensitivityChanged);
	}

	if (AimMouseSensitivitySlider)
	{
		AimMouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleAimMouseSensitivityChanged);
		AimMouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UBlackoutSettingsWidget::HandleAimMouseSensitivityChanged);
	}

	if (GraphicsTabButton)
	{
		GraphicsTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleGraphicsTabClicked);
		GraphicsTabButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleGraphicsTabClicked);
	}

	if (AudioTabButton)
	{
		AudioTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleAudioTabClicked);
		AudioTabButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleAudioTabClicked);
	}

	if (InputTabButton)
	{
		InputTabButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleInputTabClicked);
		InputTabButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleInputTabClicked);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
		ApplyButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleResetClicked);
		ResetButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleResetClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleCloseClicked);
	}
}

void UBlackoutSettingsWidget::UpdateControlState()
{
	const bool bDLSSAvailable = UBlackoutGraphicsBlueprintLibrary::IsDLSSRuntimeAvailable();
	const bool bFrameGenerationAvailable = UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationRuntimeAvailable();
	const bool bReflexAvailable = UBlackoutGraphicsBlueprintLibrary::IsReflexRuntimeAvailable();
	const bool bNeedsDLSSMode = PendingUpscalerMode == EBlackoutUpscalerMode::DLSS;

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->SetIsEnabled(bDLSSAvailable && bNeedsDLSSMode);
	}

	if (ReflexComboBox)
	{
		ReflexComboBox->SetIsEnabled(bReflexAvailable);
	}

	if (FrameGenerationComboBox)
	{
		FrameGenerationComboBox->SetIsEnabled(bFrameGenerationAvailable);
	}

	if (RuntimeAvailabilityText)
	{
		const FText AvailabilityText = FText::Format(
			NSLOCTEXT("BlackoutSettings", "AvailabilityFormat", "DLSS: {0}  |  프레임 생성: {1}  |  Reflex: {2}"),
			bDLSSAvailable
				? NSLOCTEXT("BlackoutSettings", "Available", "사용 가능")
				: NSLOCTEXT("BlackoutSettings", "Unavailable", "사용 불가"),
			bFrameGenerationAvailable
				? NSLOCTEXT("BlackoutSettings", "FrameGenerationAvailable", "사용 가능")
				: NSLOCTEXT("BlackoutSettings", "FrameGenerationUnavailable", "사용 불가"),
			bReflexAvailable
				? NSLOCTEXT("BlackoutSettings", "ReflexAvailable", "사용 가능")
				: NSLOCTEXT("BlackoutSettings", "ReflexUnavailable", "사용 불가"));
		RuntimeAvailabilityText->SetText(AvailabilityText);
	}

	UpdateTabButtonState();
}

void UBlackoutSettingsWidget::UpdateValueTexts()
{
	if (MasterVolumeValueText)
	{
		MasterVolumeValueText->SetText(FText::AsPercent(PendingMasterVolume));
	}

	if (MusicVolumeValueText)
	{
		MusicVolumeValueText->SetText(FText::AsPercent(PendingMusicVolume));
	}

	if (SFXVolumeValueText)
	{
		SFXVolumeValueText->SetText(FText::AsPercent(PendingSFXVolume));
	}

	if (MouseSensitivityValueText)
	{
		MouseSensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"), PendingMouseSensitivity)));
	}

	if (AimMouseSensitivityValueText)
	{
		AimMouseSensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"), PendingAimMouseSensitivityMultiplier)));
	}
}

void UBlackoutSettingsWidget::SelectSettingsTab(const EBlackoutSettingsTab InTab)
{
	ActiveSettingsTab = InTab;

	if (SettingsContentSwitcher)
	{
		int32 ActiveWidgetIndex = 0;
		switch (ActiveSettingsTab)
		{
		case EBlackoutSettingsTab::Audio:
			ActiveWidgetIndex = 1;
			break;

		case EBlackoutSettingsTab::Input:
			ActiveWidgetIndex = 2;
			break;

		case EBlackoutSettingsTab::Graphics:
		default:
			ActiveWidgetIndex = 0;
			break;
		}

		if (SettingsContentSwitcher->GetNumWidgets() > ActiveWidgetIndex)
		{
			SettingsContentSwitcher->SetActiveWidgetIndex(ActiveWidgetIndex);
		}
	}

	UpdateTabButtonState();
}

void UBlackoutSettingsWidget::UpdateTabButtonState()
{
	if (GraphicsTabButton)
	{
		GraphicsTabButton->SetIsEnabled(ActiveSettingsTab != EBlackoutSettingsTab::Graphics);
	}

	if (AudioTabButton)
	{
		AudioTabButton->SetIsEnabled(ActiveSettingsTab != EBlackoutSettingsTab::Audio);
	}

	if (InputTabButton)
	{
		InputTabButton->SetIsEnabled(ActiveSettingsTab != EBlackoutSettingsTab::Input);
	}
}

void UBlackoutSettingsWidget::ApplyPendingSettings()
{
	UBlackoutGraphicsUserSettings* GraphicsSettings = UBlackoutGraphicsBlueprintLibrary::GetBlackoutGraphicsUserSettings();
	if (!GraphicsSettings)
	{
		return;
	}

	GraphicsSettings->SetUpscalerMode(PendingUpscalerMode);
	GraphicsSettings->SetDLSSQualityMode(PendingDLSSMode);
	GraphicsSettings->SetFrameGenerationMode(PendingFrameGenerationMode);
	GraphicsSettings->SetReflexModeOption(PendingReflexMode);
	GraphicsSettings->SetMasterVolume(PendingMasterVolume);
	GraphicsSettings->SetMusicVolume(PendingMusicVolume);
	GraphicsSettings->SetSFXVolume(PendingSFXVolume);
	GraphicsSettings->SetMouseSensitivity(PendingMouseSensitivity);
	GraphicsSettings->SetAimMouseSensitivityMultiplier(PendingAimMouseSensitivityMultiplier);
	GraphicsSettings->ApplyBlackoutGraphicsSettings(true);
}

void UBlackoutSettingsWidget::ResetPendingSettingsToDefaults()
{
	PendingUpscalerMode = EBlackoutUpscalerMode::TSR;
	PendingDLSSMode = EBlackoutDLSSQualityMode::Quality;
	PendingFrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;
	PendingReflexMode = EBlackoutReflexMode::Enabled;
	PendingMasterVolume = 1.0f;
	PendingMusicVolume = 1.0f;
	PendingSFXVolume = 1.0f;
	PendingMouseSensitivity = 1.0f;
	PendingAimMouseSensitivityMultiplier = 1.0f;
}

void UBlackoutSettingsWidget::CloseSettingsWidget()
{
	OnSettingsClosed.Broadcast();
	RemoveFromParent();
}

FString UBlackoutSettingsWidget::GetUpscalerOptionLabel(const EBlackoutUpscalerMode InMode)
{
	switch (InMode)
	{
	case EBlackoutUpscalerMode::DLSS:
		return TEXT("DLSS");

	case EBlackoutUpscalerMode::DLAA:
		return TEXT("DLAA");

	case EBlackoutUpscalerMode::TSR:
	default:
		return TEXT("TSR");
	}
}

FString UBlackoutSettingsWidget::GetDLSSModeOptionLabel(const EBlackoutDLSSQualityMode InMode)
{
	switch (InMode)
	{
	case EBlackoutDLSSQualityMode::Balanced:
		return TEXT("밸런스");

	case EBlackoutDLSSQualityMode::Performance:
		return TEXT("성능");

	case EBlackoutDLSSQualityMode::UltraPerformance:
		return TEXT("초고성능");

	case EBlackoutDLSSQualityMode::Quality:
	default:
		return TEXT("품질");
	}
}

FString UBlackoutSettingsWidget::GetReflexModeOptionLabel(const EBlackoutReflexMode InMode)
{
	switch (InMode)
	{
	case EBlackoutReflexMode::Enabled:
		return TEXT("켜기");

	case EBlackoutReflexMode::EnabledPlusBoost:
		return TEXT("켜기 + 부스트");

	case EBlackoutReflexMode::Disabled:
	default:
		return TEXT("끔");
	}
}

FString UBlackoutSettingsWidget::GetFrameGenerationOptionLabel(const EBlackoutFrameGenerationMode InMode)
{
	switch (InMode)
	{
	case EBlackoutFrameGenerationMode::On2X:
		return TEXT("2X");

	case EBlackoutFrameGenerationMode::On3X:
		return TEXT("3X");

	case EBlackoutFrameGenerationMode::On4X:
		return TEXT("4X");

	case EBlackoutFrameGenerationMode::Disabled:
	default:
		return TEXT("끔");
	}
}

bool UBlackoutSettingsWidget::TryParseUpscalerMode(const FString& InOption, EBlackoutUpscalerMode& OutMode)
{
	if (InOption.Equals(TEXT("DLSS")))
	{
		OutMode = EBlackoutUpscalerMode::DLSS;
		return true;
	}

	if (InOption.Equals(TEXT("DLAA")))
	{
		OutMode = EBlackoutUpscalerMode::DLAA;
		return true;
	}

	if (InOption.Equals(TEXT("TSR")))
	{
		OutMode = EBlackoutUpscalerMode::TSR;
		return true;
	}

	return false;
}

bool UBlackoutSettingsWidget::TryParseDLSSMode(const FString& InOption, EBlackoutDLSSQualityMode& OutMode)
{
	if ( InOption.Equals(TEXT("밸런스"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Balanced"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Balance"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutDLSSQualityMode::Balanced;
		return true;
	}

	if (InOption.Equals(TEXT("성능"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Performance"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutDLSSQualityMode::Performance;
		return true;
	}

	if (InOption.Equals(TEXT("초고성능"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Ultra Performance"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("UltraPerformance"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutDLSSQualityMode::UltraPerformance;
		return true;
	}

	if (InOption.Equals(TEXT("품질"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("퀄리티"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Quality"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutDLSSQualityMode::Quality;
		return true;
	}

	return false;
}

bool UBlackoutSettingsWidget::TryParseFrameGenerationMode(const FString& InOption, EBlackoutFrameGenerationMode& OutMode)
{
	if (InOption.Equals(TEXT("2X"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutFrameGenerationMode::On2X;
		return true;
	}

	if (InOption.Equals(TEXT("3X"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutFrameGenerationMode::On3X;
		return true;
	}

	if (InOption.Equals(TEXT("4X"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutFrameGenerationMode::On4X;
		return true;
	}

	if (InOption.Equals(TEXT("끔"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Disabled"), ESearchCase::IgnoreCase)
		|| InOption.Equals(TEXT("Off"), ESearchCase::IgnoreCase))
	{
		OutMode = EBlackoutFrameGenerationMode::Disabled;
		return true;
	}

	return false;
}

bool UBlackoutSettingsWidget::TryParseReflexMode(const FString& InOption, EBlackoutReflexMode& OutMode)
{
	if (InOption.Equals(TEXT("켜기")))
	{
		OutMode = EBlackoutReflexMode::Enabled;
		return true;
	}

	if (InOption.Equals(TEXT("켜기 + 부스트")))
	{
		OutMode = EBlackoutReflexMode::EnabledPlusBoost;
		return true;
	}

	if (InOption.Equals(TEXT("끔")))
	{
		OutMode = EBlackoutReflexMode::Disabled;
		return true;
	}

	return false;
}

void UBlackoutSettingsWidget::HandleUpscalerSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	EBlackoutUpscalerMode ParsedMode = PendingUpscalerMode;
	if (TryParseUpscalerMode(SelectedItem, ParsedMode))
	{
		PendingUpscalerMode = ParsedMode;
		UpdateControlState();
	}
}

void UBlackoutSettingsWidget::HandleDLSSModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	EBlackoutDLSSQualityMode ParsedMode = PendingDLSSMode;
	if (TryParseDLSSMode(SelectedItem, ParsedMode))
	{
		PendingDLSSMode = ParsedMode;
	}
}

void UBlackoutSettingsWidget::HandleReflexModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	EBlackoutReflexMode ParsedMode = PendingReflexMode;
	if (TryParseReflexMode(SelectedItem, ParsedMode))
	{
		PendingReflexMode = ParsedMode;
	}
}

void UBlackoutSettingsWidget::HandleFrameGenerationSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	EBlackoutFrameGenerationMode ParsedMode = PendingFrameGenerationMode;
	if (TryParseFrameGenerationMode(SelectedItem, ParsedMode))
	{
		PendingFrameGenerationMode = ParsedMode;
	}
}

void UBlackoutSettingsWidget::HandleMasterVolumeChanged(const float InValue)
{
	PendingMasterVolume = InValue;
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleMusicVolumeChanged(const float InValue)
{
	PendingMusicVolume = InValue;
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleSFXVolumeChanged(const float InValue)
{
	PendingSFXVolume = InValue;
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleMouseSensitivityChanged(const float InValue)
{
	PendingMouseSensitivity = FMath::Lerp(0.1f, 3.0f, InValue);
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleAimMouseSensitivityChanged(const float InValue)
{
	PendingAimMouseSensitivityMultiplier = FMath::Lerp(0.1f, 3.0f, InValue);
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleGraphicsTabClicked()
{
	SelectSettingsTab(EBlackoutSettingsTab::Graphics);
}

void UBlackoutSettingsWidget::HandleAudioTabClicked()
{
	SelectSettingsTab(EBlackoutSettingsTab::Audio);
}

void UBlackoutSettingsWidget::HandleInputTabClicked()
{
	SelectSettingsTab(EBlackoutSettingsTab::Input);
}

void UBlackoutSettingsWidget::HandleApplyClicked()
{
	ApplyPendingSettings();
	RefreshFromCurrentSettings();
}

void UBlackoutSettingsWidget::HandleResetClicked()
{
	ResetPendingSettingsToDefaults();
	SyncControlsFromPendingSettings();
	UpdateControlState();
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::HandleCloseClicked()
{
	CloseSettingsWidget();
}
