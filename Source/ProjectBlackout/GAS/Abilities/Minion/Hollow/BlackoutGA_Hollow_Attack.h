// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutMinionGameplayAbility.h"
#include "BlackoutGA_Hollow_Attack.generated.h"

/**
 * 
 */
class UGameplayEffect;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Hollow_Attack : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	float Damage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	TSubclassOf<UGameplayEffect> Effect;
	
};
