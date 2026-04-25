#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_Ravager_Howl_Phase.generated.h"

/**
 * Phase B 진입 트리거 — 붉은 안개 포효
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Howl_Phase : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_Howl_Phase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
