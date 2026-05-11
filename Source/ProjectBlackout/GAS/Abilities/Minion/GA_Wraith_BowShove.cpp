// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_BowShove.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayAnimAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/Tasks/AbilityTask_BossMeleeSweep.h"
#include "Interfaces/BlackoutDamageable.h"

UGA_Wraith_BowShove::UGA_Wraith_BowShove()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tag;
	Tag.AddTag(BlackoutGameplayTags::Ability_Wraith_BowShove);
	SetAssetTags(Tag);
}

void UGA_Wraith_BowShove::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !
		BowShoveAnimMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BowShoveAnimMontage);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	MontageTask->OnCompleted.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Enemy_Attack_SweepStart, nullptr,
			true, true
		);

	if (WaitStartTask)
	{
		WaitStartTask->EventReceived.AddDynamic(
			this, &UGA_Wraith_BowShove::OnSweepStartEvent);
		WaitStartTask->ReadyForActivation();
	}
}

void UGA_Wraith_BowShove::OnSweepStartEvent(FGameplayEventData Payload)
{
	// 휘두름 Cue
	if (UAbilitySystemComponent* SourceASC =
		GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParameters;
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			CueParameters.Location = Avatar->GetActorLocation();
			CueParameters.Normal = Avatar->GetActorForwardVector();
		}
		SourceASC->ExecuteGameplayCue(
			BlackoutGameplayTags::GameplayCue_Wraith_BowShove, CueParameters);
	}

	// 활대 스윕 시작
	ActiveSweepTask = UAbilityTask_BossMeleeSweep::CreateSweepTask(
		this, StartSocketName, EndSocketName, SweepRadius);
	if (ActiveSweepTask)
	{
		ActiveSweepTask->OnHit.AddDynamic(
			this, &UGA_Wraith_BowShove::OnMeleeSweepHit);
		ActiveSweepTask->ReadyForActivation();
	}

	// 스윕 끝 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitEndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Enemy_Attack_SweepEnd, nullptr,
			true, true);
	
	if (WaitEndTask)
	{
		WaitEndTask->EventReceived.AddDynamic(this , &UGA_Wraith_BowShove::OnSweepEndEvent);
		WaitEndTask->ReadyForActivation();
	}
}

void UGA_Wraith_BowShove::OnSweepEndEvent(FGameplayEventData Payload)
{
	if (ActiveSweepTask)
	{
		ActiveSweepTask->EndTask();
		ActiveSweepTask=nullptr;
	}
}

void UGA_Wraith_BowShove::OnMeleeSweepHit(const FHitResult& HitResult)
{
	if (!DamageEffectClass)
	{
		return;
	}
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitResult.GetActor());
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
	
	FGameplayEffectSpecHandle SpecHandle= OwnerASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data -> SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage , DamageMagnitude);
		Damageable->ReceiveDamageFromHitbox(SpecHandle,HitResult.BoneName);
	}
}


void UGA_Wraith_BowShove::OnMontageEnded()
{
	if (ActiveSweepTask)
	{
		ActiveSweepTask->EndTask();
		ActiveSweepTask=nullptr;
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo,CurrentActivationInfo,true,false);
}
