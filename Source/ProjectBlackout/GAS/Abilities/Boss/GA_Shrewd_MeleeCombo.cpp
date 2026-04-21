#include "GAS/Abilities/Boss/GA_Shrewd_MeleeCombo.h"

UGA_Shrewd_MeleeCombo::UGA_Shrewd_MeleeCombo()
{
}

void UGA_Shrewd_MeleeCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 근접 타격 콤보 몽타주 및 충돌 체크
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
