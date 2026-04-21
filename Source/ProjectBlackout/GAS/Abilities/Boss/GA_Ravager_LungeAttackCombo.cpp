#include "GAS/Abilities/Boss/GA_Ravager_LungeAttackCombo.h"

UGA_Ravager_LungeAttackCombo::UGA_Ravager_LungeAttackCombo()
{
}

void UGA_Ravager_LungeAttackCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 도약 덮침 -> 할퀴기 -> 물기 콤보 처리 및 기둥 충돌 시 Multicast_Shatter 호출
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
