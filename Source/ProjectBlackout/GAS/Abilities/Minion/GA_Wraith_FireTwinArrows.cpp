// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_FireTwinArrows.h"

UGA_Wraith_FireTwinArrows::UGA_Wraith_FireTwinArrows()
{
}

void UGA_Wraith_FireTwinArrows::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TODO: 활시위 당김 모션 + 2연발 발사 (IntervalSeconds 간격) + GameplayCue.Wraith.Fire 트리거 + 화살 projectile spawn
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
