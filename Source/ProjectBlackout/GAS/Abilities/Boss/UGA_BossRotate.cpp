// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/UGA_BossRotate.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "BORavagerBoss.h"
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
	
	if (To180Threshold <= AbsAngle)
	{
		Montage = (SignedAngle > 0) ? RightTurn180 : LeftTurn180;
	}
	else if (To135Threshold <= AbsAngle)
	{
		Montage = (SignedAngle > 0) ? RightTurn135 : LeftTurn135;
	}
	else if (To90Threshold <= AbsAngle)
	{
		Montage = (SignedAngle > 0) ? RightTurn90 : LeftTurn90;
	}
	
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());
	if (!Boss || !Boss->MotionWarpingComponent)
	{
		return;
	}
	
	if (Target && Boss->MotionWarpingComponent)
	{
		Boss->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
			WarpTargetName,
			Target->GetRootComponent(),
			NAME_None,
			true,
			FVector::ZeroVector
		);
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
