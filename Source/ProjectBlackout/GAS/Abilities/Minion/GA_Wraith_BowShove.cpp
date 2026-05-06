// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_BowShove.h"

UGA_Wraith_BowShove::UGA_Wraith_BowShove()
{
}

void UGA_Wraith_BowShove::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 활 휘드름 모션 + GameplayCue.Wraith.BowShove 트리거 + GE_Wraith_Shove (knockback + 경직)
	EndAbility(Handle , ActorInfo , ActivationInfo, true , false);
}
