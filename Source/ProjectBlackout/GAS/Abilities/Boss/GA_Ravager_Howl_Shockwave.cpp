#include "GAS/Abilities/Boss/GA_Ravager_Howl_Shockwave.h"

UGA_Ravager_Howl_Shockwave::UGA_Ravager_Howl_Shockwave()
{
}

void UGA_Ravager_Howl_Shockwave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 광역 충격파 포효 연출 및 데미지 판정, Phase C 진입
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
