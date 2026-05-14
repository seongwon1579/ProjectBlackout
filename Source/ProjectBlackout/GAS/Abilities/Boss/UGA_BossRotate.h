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
	UAnimMontage* LeftTurn90;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RightTurn90;
	
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* LeftTurn135;
	
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RightTurn135;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* LeftTurn180;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RightTurn180;
	
	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	FName WarpTargetName = TEXT("MW_Target");
	
	UPROPERTY(EditAnywhere, Category = "Condition")
	float To90Threshold = 25.f;
	
	UPROPERTY(EditAnywhere, Category = "Condition")
	float To135Threshold = 90.f;
	
	UPROPERTY(EditAnywhere, Category = "Condition")
	float To180Threshold = 135.f;
	
	UFUNCTION()
	void OnMontageEnded();
};
