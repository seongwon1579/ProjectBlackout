#include "BlackoutAbilitySystemComponent.h"
#include "Abilities/BlackoutGameplayAbility.h"
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

		FGameplayAbilitySpec Spec(AbilityClass, 1);
		
		// 만약 UBlackoutGameplayAbility라면 정의된 InputID를 Spec에 할당
		if (const UBlackoutGameplayAbility* BlackoutAbility = Cast<UBlackoutGameplayAbility>(AbilityClass->GetDefaultObject()))
		{
			if (BlackoutAbility->InputID != EBlackoutAbilityInputID::None)
			{
				Spec.InputID = static_cast<int32>(BlackoutAbility->InputID);
			}
		}

		GiveAbility(Spec);
		BO_LOG_GAS(Verbose, "Granted ability: %s (InputID: %d)", *AbilityClass->GetName(), Spec.InputID);
	}
}

void UBlackoutAbilitySystemComponent::HandleAbilityInputPressed(EBlackoutAbilityInputID InputID)
{
	if (InputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	const int32 RawInputID = static_cast<int32>(InputID);
	bool bFoundMatchingAbility = false;

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID != RawInputID)
		{
			continue;
		}

		bFoundMatchingAbility = true;
		AbilitySpec.InputPressed = true;

		if (AbilitySpec.IsActive())
		{
			AbilitySpecInputPressed(AbilitySpec);
			continue;
		}

		const bool bActivated = TryActivateAbility(AbilitySpec.Handle);
		BO_LOG_GAS(Verbose, "InputPressed: ID=%d Ability=%s Activated=%s", RawInputID, *GetNameSafe(AbilitySpec.Ability), bActivated ? TEXT("true") : TEXT("false"));
	}

	if (!bFoundMatchingAbility)
	{
		BO_LOG_GAS(Verbose, "InputPressed: ID=%d has no matching ability on %s", RawInputID, *GetNameSafe(GetOwner()));
	}
}

void UBlackoutAbilitySystemComponent::HandleAbilityInputReleased(EBlackoutAbilityInputID InputID)
{
	if (InputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	const int32 RawInputID = static_cast<int32>(InputID);

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID != RawInputID)
		{
			continue;
		}

		AbilitySpec.InputPressed = false;

		if (AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
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
