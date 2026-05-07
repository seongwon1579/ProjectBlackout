#include "GAS/Abilities/Player/BlackoutGA_UseBloodRoot.h"

#include "Core/BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"

bool UBlackoutGA_UseBloodRoot::ApplyConsumableEffect(const UBOConsumableData* UsedConsumableData)
{
	UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!UsedConsumableData || !BlackoutAbilitySystemComponent)
	{
		return false;
	}

	const float HealAmountPerTick = GetEffectMagnitudeOrDefault(
		UsedConsumableData,
		BlackoutGameplayTags::Data_Consumable_HealAmount,
		0.0f);
	if (HealAmountPerTick <= 0.0f)
	{
		BO_LOG_GAS(Warning, "블러드 루트 회복 실패: HealAmount가 0 이하입니다. Data=%s", *GetNameSafe(UsedConsumableData));
		return false;
	}

	const float Duration = GetEffectMagnitudeOrDefault(
		UsedConsumableData,
		BlackoutGameplayTags::Data_Consumable_Duration,
		0.0f);

	BlackoutAbilitySystemComponent->ApplyHealthRegenOverTime(HealAmountPerTick, Duration, HealTickInterval);
	return false;
}

bool UBlackoutGA_UseBloodRoot::ShouldApplyConfiguredGameplayEffect(const UBOConsumableData*) const
{
	// 블러드 루트의 실제 회복은 ASC 지속 회복 타이머가 담당합니다.
	return false;
}
