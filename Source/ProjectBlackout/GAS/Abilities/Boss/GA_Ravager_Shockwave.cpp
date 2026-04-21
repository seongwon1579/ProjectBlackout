#include "GAS/Abilities/Boss/GA_Ravager_Shockwave.h"

UGA_Ravager_Shockwave::UGA_Ravager_Shockwave()
{
}

void UGA_Ravager_Shockwave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 충전 연출 후 장풍 발사 및 기둥 충돌 시 Multicast_Shatter 유발
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
