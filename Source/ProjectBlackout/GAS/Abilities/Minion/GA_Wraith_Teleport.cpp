// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_Teleport.h"

UGA_Wraith_Teleport::UGA_Wraith_Teleport()
{
}

void UGA_Wraith_Teleport::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TODO: EQS 결과 위치로 SetActorLocation + GameplayCue.Wraith.Teleport.Start/End + State.Wraith.Invulnerable 태그 부여/해제
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
