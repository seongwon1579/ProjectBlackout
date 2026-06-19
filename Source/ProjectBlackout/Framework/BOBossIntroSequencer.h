// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 보스 레벨 진입 시 인트로 시퀀스 서버 권위 등록·재생 + 컷신 중 입력·이동 잠금·HUD 숨김
//  - 허혁: 보스 인트로 컷신 전용 BGM 연동(트리거 타이밍 분기)
// ──────────────────────────────────────

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

	/** 보스 처치 시 현재 BGM을 정리하고 전용 아웃트로를 재생합니다. */
	void PlayOutroMusic();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayerCutscene();
	
	void PlayIntroMusic();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndCutscene();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOutroMusic();
	
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

	/** 보스 처치 시 현재 BGM을 마무리하며 재생할 전용 아웃트로입니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio")
	TSoftObjectPtr<USoundBase> OutroMusic;

	/** 같은 시퀀서 클래스를 쓰더라도 보스별로 BGM 시작 타이밍을 다르게 설정합니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio")
	EBlackoutBossIntroMusicTrigger IntroMusicTrigger = EBlackoutBossIntroMusicTrigger::OnPlayBossIntro;

	/** 아웃트로가 시작되기 전 기존 전투 BGM을 줄이는 시간입니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio", meta = (ClampMin = "0.0"))
	float OutroFadeOutDuration = 0.75f;

	/** 아웃트로 진입 시 추가로 걸 페이드 인 시간입니다. */
	UPROPERTY(EditInstanceOnly, Category = "Blackout|Cutscene|Audio", meta = (ClampMin = "0.0"))
	float OutroFadeInDuration = 0.0f;
	
private:
	FTimerHandle CutsceneTimerHandle;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Cutscene")
	bool bIsTestMode = false;

	/** 중복 호출이나 컷신/AI 활성화 경로가 겹쳐도 인트로 음악은 1회만 재생합니다. */
	bool bHasPlayedIntroMusic = false;
};
