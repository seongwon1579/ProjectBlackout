#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Interfaces/BlackoutDamageable.h"

UBlackoutHitboxComponent::UBlackoutHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Setup trace channel block, overlap ignore...
}

void UBlackoutHitboxComponent::ReceiveDamageSpec(const FGameplayEffectSpecHandle& SpecHandle)
{
	if (AActor* Owner = GetOwner())
	{
		if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Owner))
		{
			// Inject PartTag and Multiplier into SpecHandle if necessary
			Damageable->ReceiveDamageFromHitbox(SpecHandle, AttachedBoneName);
		}
	}
}
