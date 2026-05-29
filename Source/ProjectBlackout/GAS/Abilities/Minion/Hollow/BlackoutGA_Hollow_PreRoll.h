// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutMinionGameplayAbility.h"
#include "BlackoutGA_Hollow_PreRoll.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Hollow_PreRoll : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	TObjectPtr<UAnimMontage> SpawnMontage;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnMontageEnded();
	
	static const FName WarpTargetName;
};
