#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Ravager_DoubleSwipe.generated.h"

/**
 * Phase A — 2연속 할퀴기 (엇박자 콤보)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_DoubleSwipe : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_DoubleSwipe();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
