#include "Framework/BlackoutAudioSettings.h"

#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

UBlackoutAudioSettings* UBlackoutAudioSettings::GetBlackoutAudioSettings()
{
	return GetMutableDefault<UBlackoutAudioSettings>();
}

bool UBlackoutAudioSettings::HasRequiredAudioAssets() const
{
	return !SettingsSoundMix.IsNull()
		&& !MasterSoundClass.IsNull()
		&& !MusicSoundClass.IsNull()
		&& !SFXSoundClass.IsNull();
}

void UBlackoutAudioSettings::SetAudioMixAssets(
	USoundMix* InSettingsSoundMix,
	USoundClass* InMasterSoundClass,
	USoundClass* InMusicSoundClass,
	USoundClass* InSFXSoundClass,
	const bool bSaveConfig)
{
	SettingsSoundMix = InSettingsSoundMix;
	MasterSoundClass = InMasterSoundClass;
	MusicSoundClass = InMusicSoundClass;
	SFXSoundClass = InSFXSoundClass;

	if (bSaveConfig)
	{
		SaveConfig();
	}
}
