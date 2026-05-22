// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Base.h"
#include "GA_Ravager_Rotate.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Rotate : public UGA_Ravager_Base
{
	GENERATED_BODY()

public:
	virtual void PreActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual FGameplayTag SelectMontageTag(
		const FGameplayEventData* TriggerEventData) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To90Threshold = 25.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To135Threshold = 90.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To180Threshold = 135.f;
	
};
