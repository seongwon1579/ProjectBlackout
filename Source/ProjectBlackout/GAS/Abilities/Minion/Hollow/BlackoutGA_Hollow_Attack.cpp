// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Minion/Hollow/BlackoutGA_Hollow_Attack.h"

#include "AbilitySystemComponent.h"
#include "BlackoutDamageable.h"
#include "BlackoutGameplayTags.h"

void UBlackoutGA_Hollow_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	
	AActor* Target = const_cast<AActor*>(TriggerEventData->Target.Get());
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Target);
	if (!Damageable)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC) return;
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(Effect, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Damage);
		Damageable->ReceiveDamageFromHitbox(SpecHandle, NAME_None);
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
