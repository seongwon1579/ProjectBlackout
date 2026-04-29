// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_BossTest.generated.h"

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

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnMontageEnded();
	
	UFUNCTION()
	void OnMontageInterrupted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	FName WarpTargetName = TEXT("MW_Target");
	
	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	float WarpReferenceDistance = 400.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	float WarpMinPlayRate = 0.3f;
	
};
