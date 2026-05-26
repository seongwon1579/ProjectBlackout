// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_HitboxAttack.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BOWeaponDebugUtils.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tasks/AbilityTask_BossMeleeHitbox.h"


void UBlackoutGA_Ravager_HitboxAttack::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	CacheHitboxComponents();
}

void UBlackoutGA_Ravager_HitboxAttack::SetupEventListeners()
{
	WaitCollisionOnEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OnCollision,
		nullptr,
		true);

	WaitCollisionOnEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_HitboxAttack::HandleHitboxEnable);
	WaitCollisionOnEvent->ReadyForActivation();

	WaitCollisionOffEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OffCollision,
		nullptr,
		true);
	WaitCollisionOffEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_HitboxAttack::HandleHitboxDisable);
	WaitCollisionOffEvent->ReadyForActivation();
}

void UBlackoutGA_Ravager_HitboxAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearHitboxTasks();
	
	if (WaitCollisionOnEvent) WaitCollisionOnEvent->EndTask();
	if (WaitCollisionOffEvent) WaitCollisionOffEvent->EndTask();

	CachedHitboxComponents.Empty();
	DamagedActorsThisWindow.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const TArray<FName>& UBlackoutGA_Ravager_HitboxAttack::GetHitboxComponentNames() const
{
	static const TArray<FName> EmptyHitboxNames;
	return EmptyHitboxNames;
}

bool UBlackoutGA_Ravager_HitboxAttack::HasValidSettings() const
{
	return CachedPatternData->BasicAttackSettings.IsValid();
}

void UBlackoutGA_Ravager_HitboxAttack::ClearHitboxTasks()
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

void UBlackoutGA_Ravager_HitboxAttack::CacheHitboxComponents()
{
	
	CachedHitboxComponents.Empty();
	if (!CachedOwner) return;

	const TArray<FName>& TargetNames = GetHitboxComponentNames();
	if (TargetNames.Num() == 0) return;

	for (UActorComponent* Comp : CachedOwner->GetComponents())
	{
		if (Comp && TargetNames.Contains(Comp->GetFName()))
		{
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Comp))
			{
				CachedHitboxComponents.Add(PrimComp);
			}
		}
	}
}

void UBlackoutGA_Ravager_HitboxAttack::HandleHitboxEnable(FGameplayEventData Payload)
{
	if (!CanActivatePattern()) return;

	HandleHitboxDisable(Payload);

	for (const TWeakObjectPtr<UPrimitiveComponent>& HitboxPtr : CachedHitboxComponents)
	{
		if (!HitboxPtr.IsValid()) continue;

		UAbilityTask_BossMeleeHitbox* Task = UAbilityTask_BossMeleeHitbox::InitCustomTask(this, HitboxPtr.Get());
		Task->OnHit.AddDynamic(this, &UBlackoutGA_Ravager_HitboxAttack::ApplyHitboxDamage);
		Task->ReadyForActivation();
		ActiveHitboxTasks.Add(Task);
	}
}

void UBlackoutGA_Ravager_HitboxAttack::ApplyHitboxDamage(const FHitResult& HitResult)
{
	if (!CanActivatePattern()) return;

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	if (DamagedActorsThisWindow.Contains(HitActor)) return;

	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor);
	if (!Damageable) return;
	
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC) return;
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(GetDamageEffect(), GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, GetDamageMagnitude());
		Damageable->ReceiveDamageFromHitbox(SpecHandle, HitResult.BoneName);
		
		DamagedActorsThisWindow.Add(HitActor);
	}
	
	if (ShouldClearHitboxOnHit())
	{
		ClearHitboxTasks();
	}
}

void UBlackoutGA_Ravager_HitboxAttack::HandleHitboxDisable(FGameplayEventData Payload)
{
	ClearHitboxTasks();
	DamagedActorsThisWindow.Empty();
}
