#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "BlackoutGraphicsUserSettings.generated.h"

UENUM(BlueprintType)
enum class EBlackoutUpscalerMode : uint8
{
	TSR UMETA(DisplayName = "TSR"),
	DLSS UMETA(DisplayName = "DLSS"),
	DLAA UMETA(DisplayName = "DLAA")
};

UENUM(BlueprintType)
enum class EBlackoutDLSSQualityMode : uint8
{
	Quality UMETA(DisplayName = "Quality"),
	Balanced UMETA(DisplayName = "Balanced"),
	Performance UMETA(DisplayName = "Performance"),
	UltraPerformance UMETA(DisplayName = "Ultra Performance")
};

UENUM(BlueprintType)
enum class EBlackoutReflexMode : uint8
{
	Disabled UMETA(DisplayName = "Disabled"),
	Enabled UMETA(DisplayName = "Enabled"),
	EnabledPlusBoost UMETA(DisplayName = "Enabled + Boost")
};

/**
 * 그래픽/오디오/마우스 감도 사용자 설정을 저장하고 적용하는 클래스입니다.
 */
UCLASS(Config = GameUserSettings, ConfigDoNotCheckDefaults)
class PROJECTBLACKOUT_API UBlackoutGraphicsUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	virtual void SetToDefaults() override;
	virtual void ValidateSettings() override;
	virtual void ApplyNonResolutionSettings() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Graphics")
	void ApplyBlackoutGraphicsSettings(bool bSaveSettings = true);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Graphics")
	void SetUpscalerMode(EBlackoutUpscalerMode InUpscalerMode);

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	EBlackoutUpscalerMode GetUpscalerMode() const { return UpscalerMode; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Graphics")
	void SetDLSSQualityMode(EBlackoutDLSSQualityMode InDLSSQualityMode);

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	EBlackoutDLSSQualityMode GetDLSSQualityMode() const { return DLSSQualityMode; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Graphics")
	void SetReflexModeOption(EBlackoutReflexMode InReflexMode);

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	EBlackoutReflexMode GetReflexModeOption() const { return ReflexMode; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsDLSSRuntimeAvailable();

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsReflexRuntimeAvailable();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void SetMasterVolume(float InMasterVolume);

	UFUNCTION(BlueprintPure, Category = "Blackout|Audio")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void SetMusicVolume(float InMusicVolume);

	UFUNCTION(BlueprintPure, Category = "Blackout|Audio")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void SetSFXVolume(float InSFXVolume);

	UFUNCTION(BlueprintPure, Category = "Blackout|Audio")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Input")
	void SetMouseSensitivity(float InMouseSensitivity);

	UFUNCTION(BlueprintPure, Category = "Blackout|Input")
	float GetMouseSensitivity() const { return MouseSensitivity; }

protected:
	/** 현재 프로젝트에서 사용할 업스케일러 종류입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics")
	EBlackoutUpscalerMode UpscalerMode = EBlackoutUpscalerMode::TSR;

	/** DLSS Super Resolution 사용 시 목표 해상도 비율을 결정하는 프리셋입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics")
	EBlackoutDLSSQualityMode DLSSQualityMode = EBlackoutDLSSQualityMode::Quality;

	/** NVIDIA Reflex의 지연 감소 모드입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics")
	EBlackoutReflexMode ReflexMode = EBlackoutReflexMode::Enabled;

	/** TSR 사용 시 적용할 Screen Percentage입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|ScreenPercentage", meta = (ClampMin = "25.0", ClampMax = "200.0"))
	float TSRScreenPercentage = 100.0f;

	/** DLSS Quality 프리셋의 Screen Percentage입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|ScreenPercentage", meta = (ClampMin = "25.0", ClampMax = "200.0"))
	float DLSSQualityScreenPercentage = 66.7f;

	/** DLSS Balanced 프리셋의 Screen Percentage입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|ScreenPercentage", meta = (ClampMin = "25.0", ClampMax = "200.0"))
	float DLSSBalancedScreenPercentage = 58.3f;

	/** DLSS Performance 프리셋의 Screen Percentage입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|ScreenPercentage", meta = (ClampMin = "25.0", ClampMax = "200.0"))
	float DLSSPerformanceScreenPercentage = 50.0f;

	/** DLSS Ultra Performance 프리셋의 Screen Percentage입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|ScreenPercentage", meta = (ClampMin = "25.0", ClampMax = "200.0"))
	float DLSSUltraPerformanceScreenPercentage = 33.3f;

	/** DLSS/DLAA/TSR 공통으로 사용할 Temporal 업스케일러용 AA 방식입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|Compatibility")
	int32 TemporalUpscalerAntiAliasingMethod = 4;

	/** Temporal 업스케일러 경로를 유지하기 위한 CVar 값입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Graphics|Compatibility")
	int32 TemporalUpscalerEnabled = 1;

	/** 전체 볼륨 슬라이더 값입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	/** 배경 음악 볼륨 슬라이더 값입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 1.0f;

	/** 효과음 볼륨 슬라이더 값입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	/** 마우스 감도 배율입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Input", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MouseSensitivity = 1.0f;

private:
	void ApplyUpscalerSettings();
	void ApplyReflexSettings() const;
	void ApplyAudioSettings() const;
	float ResolveTargetScreenPercentage() const;
	static UWorld* ResolveSettingsWorld();

	static bool HasConsoleVariable(const TCHAR* Name);
	static bool TrySetConsoleVariableInt(const TCHAR* Name, int32 Value);
	static bool TrySetConsoleVariableFloat(const TCHAR* Name, float Value);
};
