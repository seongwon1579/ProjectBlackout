#include "BlackoutCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"

ABlackoutCharacterBase::ABlackoutCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

UAbilitySystemComponent* ABlackoutCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGameplayTag ABlackoutCharacterBase::GetHitPartTag(FName BoneName) const
{
	return FGameplayTag();
}

void ABlackoutCharacterBase::ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName)
{
	if (!HasAuthority() || !AbilitySystemComponent || !SpecHandle.IsValid())
	{
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	OnHitReact();
}

void ABlackoutCharacterBase::OnDeath()
{
	BO_LOG_CORE(Log, "OnDeath: %s", *GetName());
}

void ABlackoutCharacterBase::OnHitReact()
{
}

void ABlackoutCharacterBase::OnStun()
{
}
