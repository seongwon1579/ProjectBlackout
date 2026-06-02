#include "Framework/BlackoutGraphicsUserSettings.h"

#include "Framework/BlackoutAudioSettings.h"
#include "Core/BlackoutLog.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

#if PLATFORM_WINDOWS && !UE_SERVER
#include "DLSSLibrary.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#define BLACKOUT_WITH_DLSS 1
#define BLACKOUT_WITH_STREAMLINE_DLSSG 1
#define BLACKOUT_WITH_STREAMLINE_REFLEX 1
#else
#define BLACKOUT_WITH_DLSS 0
#define BLACKOUT_WITH_STREAMLINE_DLSSG 0
#define BLACKOUT_WITH_STREAMLINE_REFLEX 0
#endif

namespace
{
	constexpr float MinSupportedScreenPercentage = 25.0f;
	constexpr float MaxSupportedScreenPercentage = 200.0f;

	const TCHAR* NGXEnableCVarName = TEXT("r.NGX.Enable");
	const TCHAR* DLSSEnableCVarName = TEXT("r.NGX.DLSS.Enable");
	const TCHAR* ScreenPercentageCVarName = TEXT("r.ScreenPercentage");
	const TCHAR* AntiAliasingMethodCVarName = TEXT("r.AntiAliasingMethod");
	const TCHAR* TemporalUpscalerCVarName = TEXT("r.TemporalAA.Upscaler");

#if BLACKOUT_WITH_DLSS
	UDLSSMode ToDLSSMode(const EBlackoutUpscalerMode UpscalerMode, const EBlackoutDLSSQualityMode QualityMode)
	{
		if (UpscalerMode == EBlackoutUpscalerMode::DLAA)
		{
			return UDLSSMode::DLAA;
		}

		switch (QualityMode)
		{
		case EBlackoutDLSSQualityMode::Balanced:
			return UDLSSMode::Balanced;

		case EBlackoutDLSSQualityMode::Performance:
			return UDLSSMode::Performance;

		case EBlackoutDLSSQualityMode::UltraPerformance:
			return UDLSSMode::UltraPerformance;

		case EBlackoutDLSSQualityMode::Quality:
		default:
			return UDLSSMode::Quality;
		}
	}
#endif

#if BLACKOUT_WITH_STREAMLINE_REFLEX
	EStreamlineReflexMode ToStreamlineReflexMode(const EBlackoutReflexMode InMode)
	{
		switch (InMode)
		{
		case EBlackoutReflexMode::Enabled:
			return EStreamlineReflexMode::Enabled;

		case EBlackoutReflexMode::EnabledPlusBoost:
			return EStreamlineReflexMode::Boost;

		case EBlackoutReflexMode::Disabled:
		default:
			return EStreamlineReflexMode::Off;
		}
	}
#endif

#if BLACKOUT_WITH_STREAMLINE_DLSSG
	EStreamlineDLSSGMode ToStreamlineDLSSGMode(const EBlackoutFrameGenerationMode InMode)
	{
		switch (InMode)
		{
		case EBlackoutFrameGenerationMode::On2X:
			return EStreamlineDLSSGMode::On2X;

		case EBlackoutFrameGenerationMode::On3X:
			return EStreamlineDLSSGMode::On3X;

		case EBlackoutFrameGenerationMode::On4X:
			return EStreamlineDLSSGMode::On4X;

		case EBlackoutFrameGenerationMode::Disabled:
		default:
			return EStreamlineDLSSGMode::Off;
		}
	}
#endif
}

void UBlackoutGraphicsUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	UpscalerMode = EBlackoutUpscalerMode::TSR;
	DLSSQualityMode = EBlackoutDLSSQualityMode::Quality;
	ReflexMode = EBlackoutReflexMode::Enabled;
	FrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;

	TSRScreenPercentage = 100.0f;
	DLSSQualityScreenPercentage = 66.7f;
	DLSSBalancedScreenPercentage = 58.3f;
	DLSSPerformanceScreenPercentage = 50.0f;
	DLSSUltraPerformanceScreenPercentage = 33.3f;

	TemporalUpscalerAntiAliasingMethod = 4;
	TemporalUpscalerEnabled = 1;

	MasterVolume = 1.0f;
	MusicVolume = 1.0f;
	SFXVolume = 1.0f;
	MouseSensitivity = 1.0f;
}

void UBlackoutGraphicsUserSettings::ValidateSettings()
{
	Super::ValidateSettings();

	TSRScreenPercentage = FMath::Clamp(TSRScreenPercentage, MinSupportedScreenPercentage, MaxSupportedScreenPercentage);
	DLSSQualityScreenPercentage = FMath::Clamp(DLSSQualityScreenPercentage, MinSupportedScreenPercentage, MaxSupportedScreenPercentage);
	DLSSBalancedScreenPercentage = FMath::Clamp(DLSSBalancedScreenPercentage, MinSupportedScreenPercentage, MaxSupportedScreenPercentage);
	DLSSPerformanceScreenPercentage = FMath::Clamp(DLSSPerformanceScreenPercentage, MinSupportedScreenPercentage, MaxSupportedScreenPercentage);
	DLSSUltraPerformanceScreenPercentage = FMath::Clamp(DLSSUltraPerformanceScreenPercentage, MinSupportedScreenPercentage, MaxSupportedScreenPercentage);

	TemporalUpscalerAntiAliasingMethod = FMath::Clamp(TemporalUpscalerAntiAliasingMethod, 0, 4);
	TemporalUpscalerEnabled = TemporalUpscalerEnabled != 0 ? 1 : 0;
	MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
	SFXVolume = FMath::Clamp(SFXVolume, 0.0f, 1.0f);
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 3.0f);
}

void UBlackoutGraphicsUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	ValidateSettings();
	ApplyUpscalerSettings();
	ApplyFrameGenerationSettings();
	ApplyReflexSettings();
	ApplyAudioSettings();
}

void UBlackoutGraphicsUserSettings::ApplyBlackoutGraphicsSettings(const bool bSaveSettings)
{
	ApplyNonResolutionSettings();

	if (bSaveSettings)
	{
		SaveSettings();
	}
}

void UBlackoutGraphicsUserSettings::SetUpscalerMode(const EBlackoutUpscalerMode InUpscalerMode)
{
	UpscalerMode = InUpscalerMode;
}

void UBlackoutGraphicsUserSettings::SetDLSSQualityMode(const EBlackoutDLSSQualityMode InDLSSQualityMode)
{
	DLSSQualityMode = InDLSSQualityMode;
}

void UBlackoutGraphicsUserSettings::SetReflexModeOption(const EBlackoutReflexMode InReflexMode)
{
	ReflexMode = InReflexMode;
}

void UBlackoutGraphicsUserSettings::SetFrameGenerationMode(const EBlackoutFrameGenerationMode InFrameGenerationMode)
{
	FrameGenerationMode = InFrameGenerationMode;
}

void UBlackoutGraphicsUserSettings::SetMasterVolume(const float InMasterVolume)
{
	MasterVolume = InMasterVolume;
}

void UBlackoutGraphicsUserSettings::SetMusicVolume(const float InMusicVolume)
{
	MusicVolume = InMusicVolume;
}

void UBlackoutGraphicsUserSettings::SetSFXVolume(const float InSFXVolume)
{
	SFXVolume = InSFXVolume;
}

void UBlackoutGraphicsUserSettings::SetMouseSensitivity(const float InMouseSensitivity)
{
	MouseSensitivity = InMouseSensitivity;
}

bool UBlackoutGraphicsUserSettings::IsDLSSRuntimeAvailable()
{
#if BLACKOUT_WITH_DLSS
	return UDLSSLibrary::IsDLSSSupported();
#else
	return false;
#endif
}

bool UBlackoutGraphicsUserSettings::IsReflexRuntimeAvailable()
{
#if BLACKOUT_WITH_STREAMLINE_REFLEX
	return UStreamlineLibraryReflex::IsReflexSupported();
#else
	return false;
#endif
}

bool UBlackoutGraphicsUserSettings::IsFrameGenerationRuntimeAvailable()
{
#if BLACKOUT_WITH_STREAMLINE_DLSSG
	return UStreamlineLibraryDLSSG::IsDLSSGSupported();
#else
	return false;
#endif
}

bool UBlackoutGraphicsUserSettings::IsFrameGenerationModeRuntimeAvailable(const EBlackoutFrameGenerationMode InFrameGenerationMode)
{
#if BLACKOUT_WITH_STREAMLINE_DLSSG
	if (InFrameGenerationMode == EBlackoutFrameGenerationMode::Disabled)
	{
		return true;
	}

	if (!UStreamlineLibraryDLSSG::IsDLSSGSupported())
	{
		return false;
	}

	return UStreamlineLibraryDLSSG::IsDLSSGModeSupported(ToStreamlineDLSSGMode(InFrameGenerationMode));
#else
	return InFrameGenerationMode == EBlackoutFrameGenerationMode::Disabled;
#endif
}

void UBlackoutGraphicsUserSettings::ApplyUpscalerSettings()
{
	const bool bUseDLSS = UpscalerMode != EBlackoutUpscalerMode::TSR;

	TrySetConsoleVariableInt(AntiAliasingMethodCVarName, TemporalUpscalerAntiAliasingMethod);
	TrySetConsoleVariableInt(TemporalUpscalerCVarName, TemporalUpscalerEnabled);

	if (!bUseDLSS)
	{
#if BLACKOUT_WITH_DLSS
		UDLSSLibrary::EnableDLSS(false);
#else
		TrySetConsoleVariableInt(DLSSEnableCVarName, 0);
		TrySetConsoleVariableInt(NGXEnableCVarName, 0);
#endif
		TrySetConsoleVariableFloat(ScreenPercentageCVarName, TSRScreenPercentage);
		return;
	}

	if (!IsDLSSRuntimeAvailable())
	{
		BO_LOG_CORE(Warning,
			"DLSS 적용을 건너뜁니다. NVIDIA DLSS 플러그인 또는 관련 CVar를 찾지 못해 TSR fallback을 사용합니다.");
#if BLACKOUT_WITH_DLSS
		UDLSSLibrary::EnableDLSS(false);
#else
		TrySetConsoleVariableInt(DLSSEnableCVarName, 0);
		TrySetConsoleVariableInt(NGXEnableCVarName, 0);
#endif
		TrySetConsoleVariableFloat(ScreenPercentageCVarName, TSRScreenPercentage);
		return;
	}

#if BLACKOUT_WITH_DLSS
	const UDLSSMode TargetMode = ToDLSSMode(UpscalerMode, DLSSQualityMode);
	if (!UDLSSLibrary::IsDLSSModeSupported(TargetMode))
	{
		BO_LOG_CORE(Warning,
			"요청한 DLSS 모드가 지원되지 않아 TSR fallback을 사용합니다. RequestedMode=%d",
			static_cast<int32>(TargetMode));
		UDLSSLibrary::EnableDLSS(false);
		TrySetConsoleVariableFloat(ScreenPercentageCVarName, TSRScreenPercentage);
		return;
	}

	
	UDLSSLibrary::SetDLSSMode(ResolveSettingsWorld(), TargetMode);
#else
	TrySetConsoleVariableInt(NGXEnableCVarName, 1);
	TrySetConsoleVariableInt(DLSSEnableCVarName, 1);
#endif
}

void UBlackoutGraphicsUserSettings::ApplyFrameGenerationSettings()
{
#if BLACKOUT_WITH_STREAMLINE_DLSSG
	if (FrameGenerationMode == EBlackoutFrameGenerationMode::Disabled)
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(EStreamlineDLSSGMode::Off);
		return;
	}

	if (!UStreamlineLibraryDLSSG::IsDLSSGSupported())
	{
		BO_LOG_CORE(Warning, "현재 하드웨어/환경에서 NVIDIA DLSS Frame Generation을 사용할 수 없어 적용을 건너뜁니다.");
		FrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;
		UStreamlineLibraryDLSSG::SetDLSSGMode(EStreamlineDLSSGMode::Off);
		return;
	}

	const EStreamlineDLSSGMode TargetMode = ToStreamlineDLSSGMode(FrameGenerationMode);
	if (TargetMode == EStreamlineDLSSGMode::Off || !UStreamlineLibraryDLSSG::IsDLSSGModeSupported(TargetMode))
	{
		BO_LOG_CORE(
			Warning,
			"요청한 DLSS Frame Generation 모드가 지원되지 않아 적용을 건너뜁니다. RequestedMode=%d",
			static_cast<int32>(TargetMode));
		FrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;
		UStreamlineLibraryDLSSG::SetDLSSGMode(EStreamlineDLSSGMode::Off);
		return;
	}

	UStreamlineLibraryDLSSG::SetDLSSGMode(TargetMode);
#else
	if (FrameGenerationMode != EBlackoutFrameGenerationMode::Disabled)
	{
		BO_LOG_CORE(Warning, "현재 빌드 대상에서는 NVIDIA DLSS Frame Generation 플러그인을 사용할 수 없습니다.");
		FrameGenerationMode = EBlackoutFrameGenerationMode::Disabled;
	}
#endif
}

void UBlackoutGraphicsUserSettings::ApplyReflexSettings() const
{
#if BLACKOUT_WITH_STREAMLINE_REFLEX
	if (!UStreamlineLibraryReflex::IsReflexSupported())
	{
		BO_LOG_CORE(Warning, "NVIDIA Streamline Reflex를 사용할 수 없어 Reflex 적용을 건너뜁니다.");
		return;
	}

	const EStreamlineReflexMode TargetMode = ToStreamlineReflexMode(ReflexMode);
	if (!UStreamlineLibraryReflex::IsReflexModeSupported(TargetMode))
	{
		BO_LOG_CORE(Warning,
			"요청한 Reflex 모드가 지원되지 않아 적용을 건너뜁니다. RequestedMode=%d",
			static_cast<int32>(TargetMode));
		return;
	}

	UStreamlineLibraryReflex::SetReflexMode(TargetMode);
#else
	BO_LOG_CORE(Warning, "현재 빌드 대상에서는 Streamline Reflex 플러그인을 사용할 수 없습니다.");
#endif
}

void UBlackoutGraphicsUserSettings::ApplyAudioSettings() const
{
	static bool bLoggedMissingAudioSetup = false;

	const UBlackoutAudioSettings* AudioSettings = GetDefault<UBlackoutAudioSettings>();
	if (!AudioSettings)
	{
		return;
	}

	UWorld* World = ResolveSettingsWorld();
	if (!World)
	{
		BO_LOG_CORE(Warning, "오디오 설정을 적용할 월드를 찾지 못했습니다.");
		return;
	}

	USoundMix* SettingsSoundMix = AudioSettings->SettingsSoundMix.LoadSynchronous();
	if (!SettingsSoundMix)
	{
		if (!bLoggedMissingAudioSetup)
		{
			BO_LOG_CORE(
				Warning,
				"오디오 옵션 적용을 건너뜁니다. Project Settings > Blackout > Audio에 SettingsSoundMix를 지정하세요.");
			bLoggedMissingAudioSetup = true;
		}
		return;
	}

	UGameplayStatics::SetBaseSoundMix(World, SettingsSoundMix);
	UGameplayStatics::PushSoundMixModifier(World, SettingsSoundMix);

	auto ApplySoundClassOverride = [World, SettingsSoundMix](USoundClass* SoundClass, const float Volume)
	{
		if (!SoundClass)
		{
			return false;
		}

		UGameplayStatics::SetSoundMixClassOverride(World, SettingsSoundMix, SoundClass, Volume, 1.0f, 0.0f, true);
		return true;
	};

	const bool bAppliedAnyClass =
		ApplySoundClassOverride(AudioSettings->MasterSoundClass.LoadSynchronous(), MasterVolume) |
		ApplySoundClassOverride(AudioSettings->MusicSoundClass.LoadSynchronous(), MusicVolume) |
		ApplySoundClassOverride(AudioSettings->SFXSoundClass.LoadSynchronous(), SFXVolume);

	if (!bAppliedAnyClass && !bLoggedMissingAudioSetup)
	{
		BO_LOG_CORE(
			Warning,
			"오디오 옵션 적용을 건너뜁니다. Project Settings > Blackout > Audio에 Master/Music/SFX SoundClass를 지정하세요.");
		bLoggedMissingAudioSetup = true;
	}
}

float UBlackoutGraphicsUserSettings::ResolveTargetScreenPercentage() const
{
	if (UpscalerMode == EBlackoutUpscalerMode::DLAA)
	{
		return 100.0f;
	}

	switch (DLSSQualityMode)
	{
	case EBlackoutDLSSQualityMode::Balanced:
		return DLSSBalancedScreenPercentage;

	case EBlackoutDLSSQualityMode::Performance:
		return DLSSPerformanceScreenPercentage;

	case EBlackoutDLSSQualityMode::UltraPerformance:
		return DLSSUltraPerformanceScreenPercentage;

	case EBlackoutDLSSQualityMode::Quality:
	default:
		return DLSSQualityScreenPercentage;
	}
}

UWorld* UBlackoutGraphicsUserSettings::ResolveSettingsWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	if (UWorld* CurrentPlayWorld = GEngine->GetCurrentPlayWorld())
	{
		return CurrentPlayWorld;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld* World = WorldContext.World())
		{
			switch (WorldContext.WorldType)
			{
			case EWorldType::PIE:
			case EWorldType::Game:
			case EWorldType::GamePreview:
				return World;

			default:
				break;
			}
		}
	}

	return nullptr;
}

bool UBlackoutGraphicsUserSettings::HasConsoleVariable(const TCHAR* Name)
{
	return IConsoleManager::Get().FindConsoleVariable(Name) != nullptr;
}

bool UBlackoutGraphicsUserSettings::TrySetConsoleVariableInt(const TCHAR* Name, const int32 Value)
{
	if (IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		ConsoleVariable->Set(Value, ECVF_SetByGameSetting);
		return true;
	}

	return false;
}

bool UBlackoutGraphicsUserSettings::TrySetConsoleVariableFloat(const TCHAR* Name, const float Value)
{
	if (IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		ConsoleVariable->Set(Value, ECVF_SetByGameSetting);
		return true;
	}

	return false;
}
