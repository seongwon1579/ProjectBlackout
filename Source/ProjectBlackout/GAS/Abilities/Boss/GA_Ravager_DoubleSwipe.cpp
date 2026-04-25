#include "GAS/Abilities/Boss/GA_Ravager_DoubleSwipe.h"

UGA_Ravager_DoubleSwipe::UGA_Ravager_DoubleSwipe()
{
}

void UGA_Ravager_DoubleSwipe::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 2연속 할퀴기 (엇박자 콤보) 몽타주 및 충돌 판정
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
