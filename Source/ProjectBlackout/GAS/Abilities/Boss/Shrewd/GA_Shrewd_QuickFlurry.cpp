#include "GAS/Abilities/Boss/Shrewd/GA_Shrewd_QuickFlurry.h"

UGA_Shrewd_QuickFlurry::UGA_Shrewd_QuickFlurry()
{
}

void UGA_Shrewd_QuickFlurry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: IntervalSeconds 간격으로 ShotCount 만큼 연속 투사체 발사
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
