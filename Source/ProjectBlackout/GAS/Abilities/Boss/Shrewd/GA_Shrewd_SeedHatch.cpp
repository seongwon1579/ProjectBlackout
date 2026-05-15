#include "GAS/Abilities/Boss/Shrewd/GA_Shrewd_SeedHatch.h"

UGA_Shrewd_SeedHatch::UGA_Shrewd_SeedHatch()
{
}

void UGA_Shrewd_SeedHatch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: (보류) 씨앗 소환 및 무적 부여
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
