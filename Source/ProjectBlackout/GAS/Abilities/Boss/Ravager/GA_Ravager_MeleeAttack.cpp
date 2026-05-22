// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Boss/Ravager/GA_Ravager_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "GAS/Tasks/AbilityTask_BossMeleeHitbox.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"

void UGA_Ravager_MeleeAttack::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Ravager_MeleeAttack::SetupEventListeners()
{
	WaitCollisionEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OnCollision,
		nullptr,
		true
	);
	WaitCollisionEvent->EventReceived.AddDynamic(this, &UGA_Ravager_MeleeAttack::OnCollision);
	WaitCollisionEvent->ReadyForActivation();
}

void UGA_Ravager_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	for (TObjectPtr<UAbilityTask_BossMeleeHitbox>& Task : ActiveHitboxTasks)
	{
		if (Task) Task->EndTask();
	}
	ActiveHitboxTasks.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Ravager_MeleeAttack::OnCollision(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("데미지 입힘"))
	
	if (!CachedOwner) return;
	if (!CachedPatternData || !CachedPatternData->MeleeSettings.IsValid()) return;
	
	const TArray<FName> HitboxComponentNames = CachedPatternData->MeleeSettings.HitboxComponentNames;

	//TODO: 추후 수정 필요
	for (const FName& CompName : HitboxComponentNames)
	{
		UPrimitiveComponent* Hitbox = nullptr;
		for (UActorComponent* Comp : CachedOwner->GetComponents())
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
				*CompName.ToString(), *CachedOwner->GetName());
			continue;
		}

		UAbilityTask_BossMeleeHitbox* Task = UAbilityTask_BossMeleeHitbox::InitCustomTask(this, Hitbox);
		Task->OnHit.AddDynamic(this, &UGA_Ravager_MeleeAttack::OffCollision);
		Task->ReadyForActivation();
		ActiveHitboxTasks.Add(Task);
	}

	UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OffCollision,
		nullptr,
		true
	);
	WaitEndTask->EventReceived.AddDynamic(this, &UGA_Ravager_MeleeAttack::EndHitBox);
	WaitEndTask->ReadyForActivation();
}

void UGA_Ravager_MeleeAttack::EndHitBox(FGameplayEventData Payload)
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

void UGA_Ravager_MeleeAttack::OffCollision(const FHitResult& Result)
{
	if (!CachedPatternData || !CachedPatternData->MeleeSettings.IsValid()) return;
	
	const auto& DamageSetting = CachedPatternData->MeleeSettings;
	
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Result.GetActor());
	if (!Damageable) return;
	
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC) return;
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle SpecHandle= OwnerASC->MakeOutgoingSpec(DamageSetting.Effect, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data -> SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage , DamageSetting.DamageMagnitude);
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

