#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLogCategories.h"

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
		UE_LOG(LogBlackoutGAS, Verbose, TEXT("Granted ability: %s"), *AbilityClass->GetName());
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
	UE_LOG(LogBlackoutGAS, Verbose, TEXT("ASC cleared: %s"), *GetOwner()->GetName());
}
