#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Ravager_EnergyBurst.generated.h"

/**
 * Phase B — 제자리 웅크려 충전 후 광역 에너지 파동
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_EnergyBurst : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_EnergyBurst();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float ChargeDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float BlastRadius = 1500.0f;
};
