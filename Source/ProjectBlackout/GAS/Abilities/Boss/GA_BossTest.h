// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GameplayEffect.h"
#include "GA_BossTest.generated.h"

struct FHitResult;
struct FGameplayEventData;
class UAbilityTask_BossMeleeSweep;

/**
 *
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_BossTest : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BossTest();

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Montage;

	/** 히트 시 적용할 데미지 GE */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 스윕 시작 소켓 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FName SweepStartSocket = TEXT("Socket_Jaw_Root");

	/** 스윕 끝 소켓 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FName SweepEndSocket = TEXT("Socket_Jaw_Tip");

	/** 스윕 구체 반지름 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float SweepRadius = 30.f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnSweepStartEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnSweepEndEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMeleeSweepHit(const FHitResult& HitResult);

	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	FName WarpTargetName = TEXT("MW_Target");

	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	float WarpReferenceDistance = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	float WarpMinPlayRate = 0.3f;

private:
	TObjectPtr<UAbilityTask_BossMeleeSweep> ActiveSweepTask;
};
