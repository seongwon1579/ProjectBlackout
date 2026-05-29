// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_Base.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UBlackoutGA_Shrewd_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	PrepareAbility();
}

void UBlackoutGA_Shrewd_Base::FinishPrepare(bool bIsSuccess)
{
	if (bIsSuccess)
	{
		SetupEventListeners();
		PlayMontage();
		return;
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UBlackoutGA_Shrewd_Base::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBlackoutGA_Shrewd_Base::PlayMontage()
{
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	
	if (!MontageTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Shrewd_Base::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UBlackoutGA_Shrewd_Base::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Shrewd_Base::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Shrewd_Base::OnMontageEnded);
	MontageTask->ReadyForActivation();
}
