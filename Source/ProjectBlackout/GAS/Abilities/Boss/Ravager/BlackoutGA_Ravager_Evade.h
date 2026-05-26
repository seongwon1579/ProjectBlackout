// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "GA_Ravager_Evade.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Evade : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()
	
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
