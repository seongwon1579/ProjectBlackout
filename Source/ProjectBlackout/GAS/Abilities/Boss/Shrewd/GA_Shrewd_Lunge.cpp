#include "GAS/Abilities/Boss/Shrewd/GA_Shrewd_Lunge.h"

UGA_Shrewd_Lunge::UGA_Shrewd_Lunge()
{
}

void UGA_Shrewd_Lunge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: Motion Warping을 이용한 대상 추적 강습 도약
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
