#include "BlackoutCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/BlackoutCollisionChannels.h"

ABlackoutCharacterBase::ABlackoutCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
}

void ABlackoutCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
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
