// ─── 구현 내역 ───────────────────────
//  - 허혁: 오디오 설정 자산 참조(사운드 클래스/믹스·메뉴/로비 BGM) 정의
//  - 김민영: 사운드 믹스 대상 에셋 지정(SetAudioMixAssets)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BlackoutAudioSettings.generated.h"

class USoundClass;
class USoundMix;
class USoundBase;

/**
 * 오디오 옵션이 제어할 사운드 에셋 참조를 보관하는 프로젝트 설정입니다.
 */
UCLASS(Config = Game, DefaultConfig, BlueprintType, meta = (DisplayName = "Blackout Audio"))
class PROJECTBLACKOUT_API UBlackoutAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 옵션 메뉴에서 사용할 공통 사운드 믹스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio")
	TSoftObjectPtr<USoundMix> SettingsSoundMix;

	/** 전체 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio")
	TSoftObjectPtr<USoundClass> MasterSoundClass;

	/** 배경 음악 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio")
	TSoftObjectPtr<USoundClass> MusicSoundClass;

	/** 효과음 볼륨 슬라이더가 제어할 사운드 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio")
	TSoftObjectPtr<USoundClass> SFXSoundClass;

	/** 타이틀 메인 메뉴에 진입했을 때 재생할 기본 BGM입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio|Music")
	TSoftObjectPtr<USoundBase> MainMenuMusic;

	/** ShelterPrep 로비 구간에서 재생할 기본 BGM입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio|Music")
	TSoftObjectPtr<USoundBase> LobbyMusic;

	/** 배경 음악을 시작할 때 사용할 기본 페이드 인 시간입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Blackout|Audio|Music", meta = (ClampMin = "0.0"))
	float BackgroundMusicFadeInDuration = 1.0f;

	/** 블루프린트에서 프로젝트 오디오 설정 객체에 접근합니다. */
	UFUNCTION(BlueprintPure, Category = "Blackout|Audio")
	static UBlackoutAudioSettings* GetBlackoutAudioSettings();

	/** SoundMix와 SoundClass 참조가 모두 지정되어 있는지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "Blackout|Audio")
	bool HasRequiredAudioAssets() const;

	/** 블루프린트에서 오디오 믹스 대상 에셋을 지정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void SetAudioMixAssets(
		USoundMix* InSettingsSoundMix,
		USoundClass* InMasterSoundClass,
		USoundClass* InMusicSoundClass,
		USoundClass* InSFXSoundClass,
		bool bSaveConfig = false);

	virtual FName GetCategoryName() const override { return TEXT("Blackout"); }
};
