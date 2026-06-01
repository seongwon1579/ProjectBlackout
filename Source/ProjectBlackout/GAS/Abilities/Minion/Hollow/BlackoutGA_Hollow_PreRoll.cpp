// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Minion/Hollow/BlackoutGA_Hollow_PreRoll.h"

#include "BlackoutGA_Hollow_Spawn.h"
#include "BORootHollow.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

const FName UBlackoutGA_Hollow_PreRoll::WarpTargetName = FName("MW_Target");

void UBlackoutGA_Hollow_PreRoll::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	
	auto Owner = Cast<ABORootHollow>(ActorInfo->AvatarActor.Get());
	if (!Owner || !Owner->MotionWarpingComponent)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const APawn* Target = Cast<const APawn>(TriggerEventData->Target.Get());
	if (!Target)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	USceneComponent* TargetRoot = Target->GetRootComponent();
	if (!TargetRoot)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	Owner->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
		WarpTargetName,
		TargetRoot,
		NAME_None,
		true,
		FVector::ZeroVector
	);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SpawnMontage,
		1.f,
		NAME_None,
		true,
		1.f
	);
	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Hollow_PreRoll::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Hollow_PreRoll::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Hollow_PreRoll::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UBlackoutGA_Hollow_PreRoll::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
