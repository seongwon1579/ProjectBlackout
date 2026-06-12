#include "Framework/BlackoutGraphicsBlueprintLibrary.h"

UBlackoutUserSettings* UBlackoutGraphicsBlueprintLibrary::GetBlackoutUserSettings()
{
	return Cast<UBlackoutUserSettings>(UGameUserSettings::GetGameUserSettings());
}

bool UBlackoutGraphicsBlueprintLibrary::ApplyBlackoutUserSettings(const bool bSaveSettings)
{
	if (UBlackoutUserSettings* UserSettings = GetBlackoutUserSettings())
	{
		UserSettings->ApplyBlackoutUserSettings(bSaveSettings);
		return true;
	}

	return false;
}

UBlackoutUserSettings* UBlackoutGraphicsBlueprintLibrary::GetBlackoutGraphicsUserSettings()
{
	return GetBlackoutUserSettings();
}

bool UBlackoutGraphicsBlueprintLibrary::ApplyBlackoutGraphicsUserSettings(const bool bSaveSettings)
{
	return ApplyBlackoutUserSettings(bSaveSettings);
}

bool UBlackoutGraphicsBlueprintLibrary::IsDLSSRuntimeAvailable()
{
	return UBlackoutUserSettings::IsDLSSRuntimeAvailable();
}

bool UBlackoutGraphicsBlueprintLibrary::IsReflexRuntimeAvailable()
{
	return UBlackoutUserSettings::IsReflexRuntimeAvailable();
}

bool UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationRuntimeAvailable()
{
	return UBlackoutUserSettings::IsFrameGenerationRuntimeAvailable();
}

bool UBlackoutGraphicsBlueprintLibrary::IsFrameGenerationModeRuntimeAvailable(const EBlackoutFrameGenerationMode InFrameGenerationMode)
{
	return UBlackoutUserSettings::IsFrameGenerationModeRuntimeAvailable(InFrameGenerationMode);
}
