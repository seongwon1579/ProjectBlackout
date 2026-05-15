#include "GAS/Abilities/Boss/Ravager/GA_Ravager_SummonMinion.h"

UGA_Ravager_SummonMinion::UGA_Ravager_SummonMinion()
{
}

void UGA_Ravager_SummonMinion::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// TODO: UBlackoutPoolSubsystem을 사용하여 SpawnCount 만큼 미니언 생성
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
