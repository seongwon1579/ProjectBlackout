#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Framework/BlackoutGraphicsUserSettings.h"
#include "BlackoutSettingsWidget.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlackoutSettingsClosed);

/**
 * 옵션 메뉴에서 그래픽, 오디오, 마우스 감도를 한 번에 다루는 설정 위젯입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Blackout|Settings")
	FOnBlackoutSettingsClosed OnSettingsClosed;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Settings")
	void RefreshFromCurrentSettings();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> UpscalerComboBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> DLSSModeComboBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ReflexComboBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> MusicVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MasterVolumeValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MusicVolumeValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SFXVolumeValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MouseSensitivityValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RuntimeAvailabilityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

private:
	void BuildFallbackWidgetTree();
	void PopulateStaticOptions();
	void BindControlEvents();
	void UpdateControlState();
	void UpdateValueTexts();
	void ApplyPendingSettings();

	static FString GetUpscalerOptionLabel(EBlackoutUpscalerMode InMode);
	static FString GetDLSSModeOptionLabel(EBlackoutDLSSQualityMode InMode);
	static FString GetReflexModeOptionLabel(EBlackoutReflexMode InMode);

	static bool TryParseUpscalerMode(const FString& InOption, EBlackoutUpscalerMode& OutMode);
	static bool TryParseDLSSMode(const FString& InOption, EBlackoutDLSSQualityMode& OutMode);
	static bool TryParseReflexMode(const FString& InOption, EBlackoutReflexMode& OutMode);

	UFUNCTION()
	void HandleUpscalerSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleDLSSModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleReflexModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleMasterVolumeChanged(float InValue);

	UFUNCTION()
	void HandleMusicVolumeChanged(float InValue);

	UFUNCTION()
	void HandleSFXVolumeChanged(float InValue);

	UFUNCTION()
	void HandleMouseSensitivityChanged(float InValue);

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleCloseClicked();

	bool bOptionsPopulated = false;
	EBlackoutUpscalerMode PendingUpscalerMode = EBlackoutUpscalerMode::TSR;
	EBlackoutDLSSQualityMode PendingDLSSMode = EBlackoutDLSSQualityMode::Quality;
	EBlackoutReflexMode PendingReflexMode = EBlackoutReflexMode::Enabled;
	float PendingMasterVolume = 1.0f;
	float PendingMusicVolume = 1.0f;
	float PendingSFXVolume = 1.0f;
	float PendingMouseSensitivity = 1.0f;
};
