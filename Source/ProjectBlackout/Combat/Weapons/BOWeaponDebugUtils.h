#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"

namespace BlackoutWeaponDebug
{
	inline const UAbilitySystemComponent* GetAbilitySystemComponent(const AActor* TargetActor)
	{
		const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(TargetActor);
		return AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	}

	inline AActor* ResolveDamageTargetActor(const FHitResult& HitResult)
	{
		if (const UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitResult.GetComponent()))
		{
			return HitboxComponent->GetOwner();
		}

		return HitResult.GetActor();
	}

	inline bool TryGetHealth(const AActor* TargetActor, float& OutHealth)
	{
		const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(TargetActor);
		if (!AbilitySystemComponent)
		{
			return false;
		}

		OutHealth = AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
		return true;
	}

	inline FColor GetHitscanDebugColor(const bool bHit, const AActor* DamageTargetActor)
	{
		if (!bHit)
		{
			return FColor::Green;
		}

		return GetAbilitySystemComponent(DamageTargetActor) ? FColor::Red : FColor::Yellow;
	}

	inline FGameplayEffectSpecHandle DuplicateGameplayEffectSpec(const FGameplayEffectSpecHandle& SourceSpecHandle)
	{
		FGameplayEffectSpecHandle DuplicatedSpecHandle;
		if (SourceSpecHandle.IsValid())
		{
			DuplicatedSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*SourceSpecHandle.Data.Get());
		}
		return DuplicatedSpecHandle;
	}
}
