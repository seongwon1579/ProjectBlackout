// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/UGA_BossRotate.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UUGA_BossRotate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const float SignedAngle = TriggerEventData->EventMagnitude;
	const APawn* Target = Cast<const APawn>(TriggerEventData->Target.Get());
	
	const float AbsAngle = FMath::Abs(SignedAngle);
	UAnimMontage* Montage = nullptr;
	
	if (AbsAngle >= LargeAngleThreshold)
		Montage = (SignedAngle > 0) ? RightTurn90 : LeftTurn90;
	else
		Montage = (SignedAngle > 0) ? RightTurn45 : LeftTurn45;
	
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f);
	Task->OnCompleted.AddDynamic(this, &UUGA_BossRotate::OnMontageEnded);
	Task->ReadyForActivation();
	
}

void UUGA_BossRotate::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UUGA_BossRotate::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
