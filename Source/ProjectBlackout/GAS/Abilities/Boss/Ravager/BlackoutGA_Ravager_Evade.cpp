// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Evade.h"

#include "BlackoutBossCharacter.h"


void UBlackoutGA_Ravager_Evade::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}