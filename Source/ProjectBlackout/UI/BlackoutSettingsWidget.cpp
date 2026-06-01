#include "UI/BlackoutSettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/BlackoutGraphicsBlueprintLibrary.h"

namespace
{
	UTextBlock* CreateTextBlock(UWidgetTree* WidgetTree, const FName Name, const FText& InText)
	{
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetText(InText);
		return TextBlock;
	}

	UHorizontalBox* CreateLabeledRow(UWidgetTree* WidgetTree, UVerticalBox* ParentBox, const FName RowName, const FText& LabelText)
	{
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), RowName);
		ParentBox->AddChildToVerticalBox(RowBox);

		UTextBlock* LabelTextBlock = CreateTextBlock(
			WidgetTree,
			*FString::Printf(TEXT("%s_Label"), *RowName.ToString()),
			LabelText);
		if (UHorizontalBoxSlot* LabelSlot = RowBox->AddChildToHorizontalBox(LabelTextBlock))
		{
			LabelSlot->SetPadding(FMargin(0.0f, 6.0f, 16.0f, 6.0f));
			LabelSlot->SetHorizontalAlignment(HAlign_Left);
			LabelSlot->SetVerticalAlignment(VAlign_Center);
			LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		return RowBox;
	}

	void AddSectionSpacer(UWidgetTree* WidgetTree, UVerticalBox* ParentBox, const float Height)
	{
		USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
		Spacer->SetSize(FVector2D(1.0f, Height));
		ParentBox->AddChildToVerticalBox(Spacer);
	}
}

TSharedRef<SWidget> UBlackoutSettingsWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		BuildFallbackWidgetTree();
	}

	return Super::RebuildWidget();
}

void UBlackoutSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PopulateStaticOptions();
	BindControlEvents();
	RefreshFromCurrentSettings();
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

	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleCloseClicked);
	}

	Super::NativeDestruct();
}

void UBlackoutSettingsWidget::RefreshFromCurrentSettings()
{
	const UBlackoutGraphicsUserSettings* GraphicsSettings = UBlackoutGraphicsBlueprintLibrary::GetBlackoutGraphicsUserSettings();
	if (!GraphicsSettings)
	{
		return;
	}

	PendingUpscalerMode = GraphicsSettings->GetUpscalerMode();
	PendingDLSSMode = GraphicsSettings->GetDLSSQualityMode();
	PendingReflexMode = GraphicsSettings->GetReflexModeOption();
	PendingMasterVolume = GraphicsSettings->GetMasterVolume();
	PendingMusicVolume = GraphicsSettings->GetMusicVolume();
	PendingSFXVolume = GraphicsSettings->GetSFXVolume();
	PendingMouseSensitivity = GraphicsSettings->GetMouseSensitivity();

	if (UpscalerComboBox)
	{
		UpscalerComboBox->SetSelectedOption(GetUpscalerOptionLabel(PendingUpscalerMode));
	}

	if (DLSSModeComboBox)
	{
		DLSSModeComboBox->SetSelectedOption(GetDLSSModeOptionLabel(PendingDLSSMode));
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

	UpdateControlState();
	UpdateValueTexts();
}

void UBlackoutSettingsWidget::BuildFallbackWidgetTree()
{
	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	WidgetTree->RootWidget = RootBorder;
	RootBorder->SetPadding(FMargin(24.0f));

	USizeBox* ModalSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ModalSizeBox"));
	ModalSizeBox->SetWidthOverride(760.0f);
	ModalSizeBox->SetHeightOverride(640.0f);
	RootBorder->SetContent(ModalSizeBox);

	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SettingsScrollBox"));
	ModalSizeBox->SetContent(ScrollBox);

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	ScrollBox->AddChild(ContentBox);

	UTextBlock* TitleText = CreateTextBlock(
		WidgetTree,
		TEXT("SettingsTitleText"),
		NSLOCTEXT("BlackoutSettings", "Title", "Options"));
	ContentBox->AddChildToVerticalBox(TitleText);

	UTextBlock* DescriptionText = CreateTextBlock(
		WidgetTree,
		TEXT("SettingsDescriptionText"),
		NSLOCTEXT("BlackoutSettings", "Description", "DLSS, 사운드, 마우스 감도를 한 번에 조정합니다."));
	DescriptionText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* DescriptionSlot = ContentBox->AddChildToVerticalBox(DescriptionText))
	{
		DescriptionSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 16.0f));
	}

	RuntimeAvailabilityText = CreateTextBlock(
		WidgetTree,
		TEXT("RuntimeAvailabilityText"),
		NSLOCTEXT("BlackoutSettings", "RuntimeAvailability", ""));
	RuntimeAvailabilityText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* AvailabilitySlot = ContentBox->AddChildToVerticalBox(RuntimeAvailabilityText))
	{
		AvailabilitySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	UTextBlock* GraphicsSectionTitle = CreateTextBlock(
		WidgetTree,
		TEXT("GraphicsSectionTitle"),
		NSLOCTEXT("BlackoutSettings", "GraphicsSection", "Graphics"));
	ContentBox->AddChildToVerticalBox(GraphicsSectionTitle);

	UHorizontalBox* UpscalerRow = CreateLabeledRow(
		WidgetTree,
		ContentBox,
		TEXT("UpscalerRow"),
		NSLOCTEXT("BlackoutSettings", "UpscalerLabel", "Upscaler"));
	UpscalerComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("UpscalerComboBox"));
	if (UHorizontalBoxSlot* ComboSlot = UpscalerRow->AddChildToHorizontalBox(UpscalerComboBox))
	{
		ComboSlot->SetHorizontalAlignment(HAlign_Fill);
		ComboSlot->SetVerticalAlignment(VAlign_Center);
		ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* DLSSRow = CreateLabeledRow(
		WidgetTree,
		ContentBox,
		TEXT("DLSSRow"),
		NSLOCTEXT("BlackoutSettings", "DLSSModeLabel", "DLSS Mode"));
	DLSSModeComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("DLSSModeComboBox"));
	if (UHorizontalBoxSlot* ComboSlot = DLSSRow->AddChildToHorizontalBox(DLSSModeComboBox))
	{
		ComboSlot->SetHorizontalAlignment(HAlign_Fill);
		ComboSlot->SetVerticalAlignment(VAlign_Center);
		ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* ReflexRow = CreateLabeledRow(
		WidgetTree,
		ContentBox,
		TEXT("ReflexRow"),
		NSLOCTEXT("BlackoutSettings", "ReflexModeLabel", "Reflex"));
	ReflexComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("ReflexComboBox"));
	if (UHorizontalBoxSlot* ComboSlot = ReflexRow->AddChildToHorizontalBox(ReflexComboBox))
	{
		ComboSlot->SetHorizontalAlignment(HAlign_Fill);
		ComboSlot->SetVerticalAlignment(VAlign_Center);
		ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	AddSectionSpacer(WidgetTree, ContentBox, 20.0f);

	UTextBlock* AudioSectionTitle = CreateTextBlock(
		WidgetTree,
		TEXT("AudioSectionTitle"),
		NSLOCTEXT("BlackoutSettings", "AudioSection", "Audio"));
	ContentBox->AddChildToVerticalBox(AudioSectionTitle);

	UTextBlock* AudioHintText = CreateTextBlock(
		WidgetTree,
		TEXT("AudioHintText"),
		NSLOCTEXT("BlackoutSettings", "AudioHint", "Music/SFX 볼륨은 Project Settings > Blackout > Audio에 SoundMix와 SoundClass를 지정해야 반영됩니다."));
	AudioHintText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* AudioHintSlot = ContentBox->AddChildToVerticalBox(AudioHintText))
	{
		AudioHintSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 12.0f));
	}

	auto BuildSliderRow = [this, ContentBox](const FName RowName, const FText& LabelText, TObjectPtr<USlider>& OutSlider, TObjectPtr<UTextBlock>& OutValueText)
	{
		UHorizontalBox* SliderRow = CreateLabeledRow(WidgetTree, ContentBox, RowName, LabelText);

		OutSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), *FString::Printf(TEXT("%s_Slider"), *RowName.ToString()));
		if (UHorizontalBoxSlot* SliderSlot = SliderRow->AddChildToHorizontalBox(OutSlider))
		{
			SliderSlot->SetHorizontalAlignment(HAlign_Fill);
			SliderSlot->SetVerticalAlignment(VAlign_Center);
			SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			SliderSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
		}

		OutValueText = CreateTextBlock(
			WidgetTree,
			*FString::Printf(TEXT("%s_Value"), *RowName.ToString()),
			FText::GetEmpty());
		if (UHorizontalBoxSlot* ValueSlot = SliderRow->AddChildToHorizontalBox(OutValueText))
		{
			ValueSlot->SetHorizontalAlignment(HAlign_Right);
			ValueSlot->SetVerticalAlignment(VAlign_Center);
		}
	};

	BuildSliderRow(TEXT("MasterVolumeRow"), NSLOCTEXT("BlackoutSettings", "MasterVolumeLabel", "Master Volume"), MasterVolumeSlider, MasterVolumeValueText);
	BuildSliderRow(TEXT("MusicVolumeRow"), NSLOCTEXT("BlackoutSettings", "MusicVolumeLabel", "Music Volume"), MusicVolumeSlider, MusicVolumeValueText);
	BuildSliderRow(TEXT("SFXVolumeRow"), NSLOCTEXT("BlackoutSettings", "SFXVolumeLabel", "SFX Volume"), SFXVolumeSlider, SFXVolumeValueText);

	AddSectionSpacer(WidgetTree, ContentBox, 20.0f);

	UTextBlock* InputSectionTitle = CreateTextBlock(
		WidgetTree,
		TEXT("InputSectionTitle"),
		NSLOCTEXT("BlackoutSettings", "InputSection", "Controls"));
	ContentBox->AddChildToVerticalBox(InputSectionTitle);

	BuildSliderRow(TEXT("MouseSensitivityRow"), NSLOCTEXT("BlackoutSettings", "MouseSensitivityLabel", "Mouse Sensitivity"), MouseSensitivitySlider, MouseSensitivityValueText);

	AddSectionSpacer(WidgetTree, ContentBox, 24.0f);

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
	if (UVerticalBoxSlot* ButtonRowSlot = ContentBox->AddChildToVerticalBox(ButtonRow))
	{
		ButtonRowSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	ApplyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ApplyButton"));
	if (UHorizontalBoxSlot* ButtonSlot = ButtonRow->AddChildToHorizontalBox(ApplyButton))
	{
		ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}
	ApplyButton->SetContent(CreateTextBlock(WidgetTree, TEXT("ApplyButtonText"), NSLOCTEXT("BlackoutSettings", "ApplyButton", "Apply")));

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	ButtonRow->AddChildToHorizontalBox(CloseButton);
	CloseButton->SetContent(CreateTextBlock(WidgetTree, TEXT("CloseButtonText"), NSLOCTEXT("BlackoutSettings", "CloseButton", "Close")));
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

	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
		ApplyButton->OnClicked.AddDynamic(this, &UBlackoutSettingsWidget::HandleApplyClicked);
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

	if (RuntimeAvailabilityText)
	{
		const FText AvailabilityText = FText::Format(
			NSLOCTEXT("BlackoutSettings", "AvailabilityFormat", "DLSS: {0}  |  Reflex: {1}"),
			bDLSSAvailable
				? NSLOCTEXT("BlackoutSettings", "Available", "Available")
				: NSLOCTEXT("BlackoutSettings", "Unavailable", "Unavailable"),
			bReflexAvailable
				? NSLOCTEXT("BlackoutSettings", "ReflexAvailable", "Available")
				: NSLOCTEXT("BlackoutSettings", "ReflexUnavailable", "Unavailable"));
		RuntimeAvailabilityText->SetText(AvailabilityText);
	}
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
	GraphicsSettings->SetReflexModeOption(PendingReflexMode);
	GraphicsSettings->SetMasterVolume(PendingMasterVolume);
	GraphicsSettings->SetMusicVolume(PendingMusicVolume);
	GraphicsSettings->SetSFXVolume(PendingSFXVolume);
	GraphicsSettings->SetMouseSensitivity(PendingMouseSensitivity);
	GraphicsSettings->ApplyBlackoutGraphicsSettings(true);
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
		return TEXT("Balanced");

	case EBlackoutDLSSQualityMode::Performance:
		return TEXT("Performance");

	case EBlackoutDLSSQualityMode::UltraPerformance:
		return TEXT("Ultra Performance");

	case EBlackoutDLSSQualityMode::Quality:
	default:
		return TEXT("Quality");
	}
}

FString UBlackoutSettingsWidget::GetReflexModeOptionLabel(const EBlackoutReflexMode InMode)
{
	switch (InMode)
	{
	case EBlackoutReflexMode::Enabled:
		return TEXT("Enabled");

	case EBlackoutReflexMode::EnabledPlusBoost:
		return TEXT("Enabled + Boost");

	case EBlackoutReflexMode::Disabled:
	default:
		return TEXT("Disabled");
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
	if (InOption.Equals(TEXT("Balanced")))
	{
		OutMode = EBlackoutDLSSQualityMode::Balanced;
		return true;
	}

	if (InOption.Equals(TEXT("Performance")))
	{
		OutMode = EBlackoutDLSSQualityMode::Performance;
		return true;
	}

	if (InOption.Equals(TEXT("Ultra Performance")))
	{
		OutMode = EBlackoutDLSSQualityMode::UltraPerformance;
		return true;
	}

	if (InOption.Equals(TEXT("Quality")))
	{
		OutMode = EBlackoutDLSSQualityMode::Quality;
		return true;
	}

	return false;
}

bool UBlackoutSettingsWidget::TryParseReflexMode(const FString& InOption, EBlackoutReflexMode& OutMode)
{
	if (InOption.Equals(TEXT("Enabled")))
	{
		OutMode = EBlackoutReflexMode::Enabled;
		return true;
	}

	if (InOption.Equals(TEXT("Enabled + Boost")))
	{
		OutMode = EBlackoutReflexMode::EnabledPlusBoost;
		return true;
	}

	if (InOption.Equals(TEXT("Disabled")))
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

void UBlackoutSettingsWidget::HandleApplyClicked()
{
	ApplyPendingSettings();
	RefreshFromCurrentSettings();
}

void UBlackoutSettingsWidget::HandleCloseClicked()
{
	OnSettingsClosed.Broadcast();
	RemoveFromParent();
}
