#include "Combat/Components/BlackoutHitboxComponent.h"

#include "Core/BlackoutCollisionChannels.h"
#include "Interfaces/BlackoutDamageable.h"

UBlackoutHitboxComponent::UBlackoutHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UBoxComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UBoxComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UBoxComponent::SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	UBoxComponent::SetGenerateOverlapEvents(false);
}

void UBlackoutHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	UBoxComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UBoxComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UBoxComponent::SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	UBoxComponent::SetGenerateOverlapEvents(false);
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
