// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Boss/GA_BossMeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "GAS/Tasks/AbilityTask_BossMeleeSweep.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"
#include "MotionWarpingComponent.h"

UGA_BossMeleeAttack::UGA_BossMeleeAttack()
{
}

void UGA_BossMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SetupMotionWarp(ActorInfo, TriggerEventData);
	PlayAttackMontage();
	WaitForSweepStart();
}

void UGA_BossMeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());
	if (Boss && Boss->MotionWarpingComponent)
	{
		Boss->MotionWarpingComponent->RemoveWarpTarget(WarpTargetName);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossMeleeAttack::SetupMotionWarp(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayEventData* TriggerEventData)
{
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());
	if (!Boss || !Boss->MotionWarpingComponent)
	{
		return;
	}

	const APawn* Target = TriggerEventData
		? Cast<const APawn>(TriggerEventData->Target.Get())
		: nullptr;
	if (!Target)
	{
		return;
	}

	Boss->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
		WarpTargetName,
		Target->GetRootComponent(),
		NAME_None,
		true,
		FVector::ZeroVector
	);
}

void UGA_BossMeleeAttack::PlayAttackMontage()
{
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.f,
		NAME_None,
		true,
		1.f
	);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_BossMeleeAttack::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGA_BossMeleeAttack::WaitForSweepStart()
{
	UAbilityTask_WaitGameplayEvent* WaitStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_SweepStart,
		nullptr,
		true
	);
	WaitStartTask->EventReceived.AddDynamic(this, &UGA_BossMeleeAttack::OnSweepStartEvent);
	WaitStartTask->ReadyForActivation();
}

void UGA_BossMeleeAttack::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BossMeleeAttack::OnMontageInterrupted()
{
}

void UGA_BossMeleeAttack::OnMontageCancelled()
{
}

void UGA_BossMeleeAttack::OnSweepStartEvent(FGameplayEventData Payload)
{
	for (const FBossSweepSocketPair& Pair : SweepSocketPairs)
	{
		UAbilityTask_BossMeleeSweep* SweepTask = UAbilityTask_BossMeleeSweep::CreateSweepTask(
			this, Pair.StartSocket, Pair.EndSocket, SweepRadius);
		SweepTask->OnHit.AddDynamic(this, &UGA_BossMeleeAttack::OnMeleeSweepHit);
		SweepTask->ReadyForActivation();
		ActiveSweepTasks.Add(SweepTask);
	}

	UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_SweepEnd,
		nullptr,
		true
	);
	WaitEndTask->EventReceived.AddDynamic(this, &UGA_BossMeleeAttack::OnSweepEndEvent);
	WaitEndTask->ReadyForActivation();
}

void UGA_BossMeleeAttack::OnSweepEndEvent(FGameplayEventData Payload)
{
	for (TObjectPtr<UAbilityTask_BossMeleeSweep>& Task : ActiveSweepTasks)
	{
		if (Task)
		{
			Task->EndTask();
		}
	}
	ActiveSweepTasks.Empty();
}

void UGA_BossMeleeAttack::OnMeleeSweepHit(const FHitResult& HitResult)
{
	if (!DamageEffectClass)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor);
	if (!Damageable)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		Damageable->ReceiveDamageFromHitbox(SpecHandle, HitResult.BoneName);
	}
}
