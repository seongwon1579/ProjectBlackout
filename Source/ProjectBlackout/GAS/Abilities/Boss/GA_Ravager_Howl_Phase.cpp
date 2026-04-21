#include "GAS/Abilities/Boss/GA_Ravager_Howl_Phase.h"

UGA_Ravager_Howl_Phase::UGA_Ravager_Howl_Phase()
{
}

void UGA_Ravager_Howl_Phase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 붉은 안개 연출 및 Phase B 전환 처리 트리거
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
