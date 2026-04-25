#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_Ravager_LungeAttackCombo.generated.h"

/**
 * Phase A — 도약 덮침 → 할퀴기 → 물기 복합 콤보
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_LungeAttackCombo : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Ravager_LungeAttackCombo();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float LungeDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	bool bBreaksPillarOnHit = true;
};
