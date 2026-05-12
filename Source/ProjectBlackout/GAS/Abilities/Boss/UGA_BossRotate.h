// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "UGA_BossRotate.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UUGA_BossRotate : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* LeftTurn45;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RightTurn45;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* LeftTurn90;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RightTurn90;
	
	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	FName WarpTargetName = TEXT("MW_Target");
	
	UPROPERTY(EditAnywhere, Category = "Condition")
	float LargeAngleThreshold = 67.5f;
	
	UFUNCTION()
	void OnMontageEnded();
};
