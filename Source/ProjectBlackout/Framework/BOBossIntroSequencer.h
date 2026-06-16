// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BOBossIntroSequencer.generated.h"

class ALevelSequenceActor;
class USoundBase;

UENUM(BlueprintType)
enum class EBlackoutBossIntroMusicTrigger : uint8
{
	/** 인트로 시퀀스 시작과 동시에 BGM을 재생합니다. */
	OnPlayBossIntro UMETA(DisplayName = "On Play Boss Intro"),

	/** 컷신 종료 후 보스 AI가 활성화되는 시점에 BGM을 재생합니다. */
	OnActivateBossAI UMETA(DisplayName = "On Activate Boss AI")
};

UCLASS()
class PROJECTBLACKOUT_API ABOBossIntroSequencer : public AActor
{
	GENERATED_BODY()

public:
	ABOBossIntroSequencer();

	void PlayBossIntro();

	void ActivateBossAI();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayerCutscene();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayIntroMusic();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndCutscene();
	
	UFUNCTION()
	void OnCutsceneTimerExpired();

public:
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene")
	ALevelSequenceActor* BossCutsceneActor;

	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene")
	AActor* TargetBoss;

	/** 이 인트로 시퀀서가 재생될 때 함께 시작할 전용 BGM입니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio")
	TSoftObjectPtr<USoundBase> IntroMusic;

	/** 같은 시퀀서 클래스를 쓰더라도 보스별로 BGM 시작 타이밍을 다르게 설정합니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio")
	EBlackoutBossIntroMusicTrigger IntroMusicTrigger = EBlackoutBossIntroMusicTrigger::OnPlayBossIntro;
	
private:
	FTimerHandle CutsceneTimerHandle;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Cutscene")
	bool bIsTestMode = false;

	/** 중복 호출이나 컷신/AI 활성화 경로가 겹쳐도 인트로 음악은 1회만 재생합니다. */
	bool bHasPlayedIntroMusic = false;
};
