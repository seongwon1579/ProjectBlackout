// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BOBossIntroSequencer.generated.h"

class ALevelSequenceActor;
class USoundBase;

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
	
private:
	FTimerHandle CutsceneTimerHandle;
};
