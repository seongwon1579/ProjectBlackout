#include "BlackoutCharacterBase.h"
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
