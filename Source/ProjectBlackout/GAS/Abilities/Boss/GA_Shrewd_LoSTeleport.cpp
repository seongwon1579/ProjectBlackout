#include "GAS/Abilities/Boss/GA_Shrewd_LoSTeleport.h"

UGA_Shrewd_LoSTeleport::UGA_Shrewd_LoSTeleport()
{
}

void UGA_Shrewd_LoSTeleport::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: UBlackboardKeyRegistry 참조하여 시야(LoS) 차단 시 점멸 후 근접 콤보 기믹 구현
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
