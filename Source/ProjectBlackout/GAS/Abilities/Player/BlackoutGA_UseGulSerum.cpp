#include "GAS/Abilities/Player/BlackoutGA_UseGulSerum.h"

#include "Data/BOConsumableData.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"

bool UBlackoutGA_UseGulSerum::ApplyConsumableEffect(const UBOConsumableData* UsedConsumableData)
{
	UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!UsedConsumableData || !BlackoutAbilitySystemComponent)
	{
		return false;
	}

	const float StaminaCostMultiplier = GetEffectMagnitudeOrDefault(
		UsedConsumableData,
		BlackoutGameplayTags::Data_Consumable_StaminaCostMultiplier,
		0.5f);
	const float Duration = GetEffectMagnitudeOrDefault(
		UsedConsumableData,
		BlackoutGameplayTags::Data_Consumable_Duration,
		60.0f);

	BlackoutAbilitySystemComponent->ApplyTemporaryStaminaCostMultiplier(StaminaCostMultiplier, Duration);
	return false;
}
