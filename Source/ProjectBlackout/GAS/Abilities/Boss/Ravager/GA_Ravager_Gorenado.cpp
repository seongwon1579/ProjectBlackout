#include "GAS/Abilities/Boss/Ravager//GA_Ravager_Gorenado.h"

UGA_Ravager_Gorenado::UGA_Ravager_Gorenado()
{
}

void UGA_Ravager_Gorenado::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: 소용돌이 연출 생성 및 PullStrength 기반으로 플레이어를 끌어당김
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
