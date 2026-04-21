#include "GAS/Abilities/Boss/GA_Ravager_EnergyBurst.h"

UGA_Ravager_EnergyBurst::UGA_Ravager_EnergyBurst()
{
}

void UGA_Ravager_EnergyBurst::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 웅크리기 충전 후 BlastRadius 범위에 광역 파동 데미지
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
