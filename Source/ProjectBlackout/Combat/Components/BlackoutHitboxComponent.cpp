#include "Combat/Components/BlackoutHitboxComponent.h"

#include "GameplayEffect.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Interfaces/BlackoutDamageable.h"

UBlackoutHitboxComponent::UBlackoutHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UCapsuleComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UCapsuleComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UCapsuleComponent::SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	UCapsuleComponent::SetGenerateOverlapEvents(false);
}

void UBlackoutHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	UCapsuleComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UCapsuleComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UCapsuleComponent::SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	UCapsuleComponent::SetGenerateOverlapEvents(false);
}

void UBlackoutHitboxComponent::ReceiveDamageSpec(const FGameplayEffectSpecHandle& SpecHandle)
{
	if (!SpecHandle.IsValid())
	{
		return;
	}

	if (PartTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(PartTag, DamageMultiplier);
	}

	if (AActor* Owner = GetOwner())
	{
		if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Owner))
		{
			Damageable->ReceiveDamageFromHitbox(SpecHandle, AttachedBoneName);
		}
	}
}
