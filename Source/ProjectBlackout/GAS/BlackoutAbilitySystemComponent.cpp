#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"

void UBlackoutAbilitySystemComponent::GiveDefaultAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			continue;
		}
		GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		BO_LOG_GAS(Verbose, "Granted ability: %s", *AbilityClass->GetName());
	}
}

void UBlackoutAbilitySystemComponent::ClearAllAbilitiesAndEffects()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	ClearAllAbilities();

	// 모든 활성 GE 제거 (풀 반환 시 상태이상/쿨다운 초기화)
	RemoveActiveEffects(FGameplayEffectQuery());
	BO_LOG_GAS(Verbose, "ASC cleared: %s", *GetOwner()->GetName());
}
