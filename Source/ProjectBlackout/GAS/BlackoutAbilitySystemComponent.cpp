#include "BlackoutAbilitySystemComponent.h"

#include "Abilities/BlackoutGameplayAbility.h"
#include "BlackoutLog.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"

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

	StopStaminaRegen();
	ClearAllAbilities();

	// 모든 활성 GE 제거 (풀 반환 시 상태이상/쿨다운 초기화)
	RemoveActiveEffects(FGameplayEffectQuery());
	BO_LOG_GAS(Verbose, "ASC cleared: %s", *GetOwner()->GetName());
}

void UBlackoutAbilitySystemComponent::NotifyStaminaSpent()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	StopStaminaRegen();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		StaminaRegenDelayTimerHandle,
		this,
		&UBlackoutAbilitySystemComponent::StartStaminaRegen,
		FMath::Max(0.0f, StaminaRegenDelay),
		false);
}

void UBlackoutAbilitySystemComponent::StartStaminaRegen()
{
	if (!CanRecoverStamina())
	{
		return;
	}

	HandleStaminaRegenTick();

	if (!CanRecoverStamina())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			StaminaRegenTickTimerHandle,
			this,
			&UBlackoutAbilitySystemComponent::HandleStaminaRegenTick,
			FMath::Max(0.01f, StaminaRegenTickInterval),
			true);
	}
}

void UBlackoutAbilitySystemComponent::HandleStaminaRegenTick()
{
	if (!CanRecoverStamina())
	{
		StopStaminaRegen();
		return;
	}

	const FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(StaminaRegenEffectClass, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		StopStaminaRegen();
		return;
	}

	ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (!CanRecoverStamina())
	{
		StopStaminaRegen();
	}
}

void UBlackoutAbilitySystemComponent::StopStaminaRegen()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaRegenDelayTimerHandle);
		World->GetTimerManager().ClearTimer(StaminaRegenTickTimerHandle);
	}
}

bool UBlackoutAbilitySystemComponent::CanRecoverStamina() const
{
	if (!IsOwnerActorAuthoritative() || !StaminaRegenEffectClass)
	{
		return false;
	}

	if (HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting))
	{
		return false;
	}

	const float CurrentStamina = GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	const float MaxStamina = GetNumericAttribute(UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute());
	return MaxStamina > 0.0f && CurrentStamina < MaxStamina;
}
