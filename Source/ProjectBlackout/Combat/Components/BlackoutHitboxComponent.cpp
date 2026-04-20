#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Interfaces/BlackoutDamageable.h"

UBlackoutHitboxComponent::UBlackoutHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UBoxComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 트레이스 채널 블록, 오버랩 무시 등 설정...
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
