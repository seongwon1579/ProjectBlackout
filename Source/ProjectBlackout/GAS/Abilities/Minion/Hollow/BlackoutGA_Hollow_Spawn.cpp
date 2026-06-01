// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Minion/Hollow/BlackoutGA_Hollow_Spawn.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UBlackoutGA_Hollow_Spawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!SpawnMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SpawnMontage,
		1.f,
		NAME_None,
		true,
		1.f
	);
	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Hollow_Spawn::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Hollow_Spawn::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Hollow_Spawn::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UBlackoutGA_Hollow_Spawn::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
