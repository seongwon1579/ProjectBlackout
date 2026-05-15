#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_Ravager_Gorenado.generated.h"

/**
 * Phase C — 궁극기 볼텍스 소용돌이 (중앙으로 흡입)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Gorenado : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_Gorenado();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float PullStrength = 1000.0f;
};
