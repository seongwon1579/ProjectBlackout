#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Ravager_Shockwave.generated.h"

/**
 * Phase A — 앞발 충전 후 지면 장풍
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Shockwave : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_Shockwave();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float ChargeDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float TravelDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	bool bBreaksPillarOnHit = true;
};
