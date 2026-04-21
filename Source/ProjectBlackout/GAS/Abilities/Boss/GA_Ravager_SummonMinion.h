#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Ravager_SummonMinion.generated.h"

/**
 * 미니언 소환 (Phase A: Root Hollow, Phase B: Root Wraith 혼합)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_SummonMinion : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_SummonMinion();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	int32 SpawnCount = 3;
};
