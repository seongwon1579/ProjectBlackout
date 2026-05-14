// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Boss/GA_BossMeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "GAS/Tasks/AbilityTask_BossMeleeHitbox.h"
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
	WaitForCollisionEvent();
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

void UGA_BossMeleeAttack::WaitForCollisionEvent()
{
	UAbilityTask_WaitGameplayEvent* WaitStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OnCollision,
		nullptr,
		true
	);
	WaitStartTask->EventReceived.AddDynamic(this, &UGA_BossMeleeAttack::OnCollision);
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

void UGA_BossMeleeAttack::OnCollision(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("데미지 입힘"))
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;
	
	//TODO: 순회로 하지 말고 Map으로 변경 필요
	for (const FName& CompName : HitboxComponentNames)
	{
		UPrimitiveComponent* Hitbox = nullptr;
		for (UActorComponent* Comp : Avatar->GetComponents())
		{
			if (Comp->GetFName() == CompName)
			{
				Hitbox = Cast<UPrimitiveComponent>(Comp);
				break;
			}
		}

		if (!Hitbox)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GA_BossMeleeAttack] Hitbox component '%s' not found on %s"),
				*CompName.ToString(), *Avatar->GetName());
			continue;
		}

		UAbilityTask_BossMeleeHitbox* Task = UAbilityTask_BossMeleeHitbox::InitCustomTask(this, Hitbox);
		Task->OnHit.AddDynamic(this, &UGA_BossMeleeAttack::OffCollision);
		Task->ReadyForActivation();
		ActiveHitboxTasks.Add(Task);
	}

	UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OffCollision,
		nullptr,
		true
	);
	WaitEndTask->EventReceived.AddDynamic(this, &UGA_BossMeleeAttack::EndHitBox);
	WaitEndTask->ReadyForActivation();
}

void UGA_BossMeleeAttack::EndHitBox(FGameplayEventData Payload)
{
	for (TObjectPtr<UAbilityTask_BossMeleeHitbox>& Task : ActiveHitboxTasks)
	{
		if (Task)
		{
			Task->EndTask();
		}
	}
	ActiveHitboxTasks.Empty();
}

void UGA_BossMeleeAttack::OffCollision(const FHitResult& Result)
{
	if (!DamageEffectClass)
	{
		return;
	}
	
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Result.GetActor());
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
		Damageable->ReceiveDamageFromHitbox(SpecHandle,Result.BoneName);
	}
	
	for (TObjectPtr<UAbilityTask_BossMeleeHitbox>& Task : ActiveHitboxTasks)
	{
		if (Task)
		{
			Task->EndTask();
		}
	}
	ActiveHitboxTasks.Empty();
}

