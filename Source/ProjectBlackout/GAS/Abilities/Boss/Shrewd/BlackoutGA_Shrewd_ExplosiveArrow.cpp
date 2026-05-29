#include "GAS/Abilities/Boss/Shrewd/GA_Shrewd_ExplosiveArrow.h"

UGA_Shrewd_ExplosiveArrow::UGA_Shrewd_ExplosiveArrow()
{
}

void UGA_Shrewd_ExplosiveArrow::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 투사체 스폰 및 폭발 시 SplashRadius 기반 광역 피해
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
