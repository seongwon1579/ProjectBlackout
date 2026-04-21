#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Ravager_Howl_Shockwave.generated.h"

/**
 * Phase C 진입 트리거 — 광역 충격파 포효
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Howl_Shockwave : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_Howl_Shockwave();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
