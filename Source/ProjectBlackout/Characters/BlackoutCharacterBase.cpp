#include "BlackoutCharacterBase.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLogCategories.h"

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
	UE_LOG(LogBlackout, Log, TEXT("OnDeath: %s"), *GetName());
}

void ABlackoutCharacterBase::OnHitReact()
{
}

void ABlackoutCharacterBase::OnStun()
{
}
