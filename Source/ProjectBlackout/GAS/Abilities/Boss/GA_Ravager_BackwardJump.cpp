#include "GAS/Abilities/Boss/GA_Ravager_BackwardJump.h"

UGA_Ravager_BackwardJump::UGA_Ravager_BackwardJump()
{
}

void UGA_Ravager_BackwardJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 뒤로 점프하는 회피 기동
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
