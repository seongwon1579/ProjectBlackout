#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_Ravager_BackwardJump.generated.h"

/**
 * Phase A — 회피 이동 (Shockwave 선행)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_BackwardJump : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_BackwardJump();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float JumpBackwardDistance = 800.0f;
};
