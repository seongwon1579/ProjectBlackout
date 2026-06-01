#include "Framework/BlackoutGraphicsBlueprintLibrary.h"

UBlackoutGraphicsUserSettings* UBlackoutGraphicsBlueprintLibrary::GetBlackoutGraphicsUserSettings()
{
	return Cast<UBlackoutGraphicsUserSettings>(UGameUserSettings::GetGameUserSettings());
}

bool UBlackoutGraphicsBlueprintLibrary::ApplyBlackoutGraphicsUserSettings(const bool bSaveSettings)
{
	if (UBlackoutGraphicsUserSettings* GraphicsSettings = GetBlackoutGraphicsUserSettings())
	{
		GraphicsSettings->ApplyBlackoutGraphicsSettings(bSaveSettings);
		return true;
	}

	return false;
}

bool UBlackoutGraphicsBlueprintLibrary::IsDLSSRuntimeAvailable()
{
	return UBlackoutGraphicsUserSettings::IsDLSSRuntimeAvailable();
}

bool UBlackoutGraphicsBlueprintLibrary::IsReflexRuntimeAvailable()
{
	return UBlackoutGraphicsUserSettings::IsReflexRuntimeAvailable();
}
