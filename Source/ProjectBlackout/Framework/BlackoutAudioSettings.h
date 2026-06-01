#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BlackoutAudioSettings.generated.h"

class USoundClass;
class USoundMix;

/**
 * 오디오 옵션이 제어할 사운드 에셋 참조를 보관하는 프로젝트 설정입니다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blackout Audio"))
class PROJECTBLACKOUT_API UBlackoutAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 옵션 메뉴에서 사용할 공통 사운드 믹스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundMix> SettingsSoundMix;

	/** 전체 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundClass> MasterSoundClass;

	/** 배경 음악 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundClass> MusicSoundClass;

	/** 효과음 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundClass> SFXSoundClass;

	virtual FName GetCategoryName() const override { return TEXT("Blackout"); }
};
