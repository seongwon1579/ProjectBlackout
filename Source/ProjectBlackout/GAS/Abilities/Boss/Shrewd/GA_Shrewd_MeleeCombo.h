#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Base.h"
#include "GA_Shrewd_MeleeCombo.generated.h"

/**
 * 지면 페이즈 — 활대 근접 타격
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_MeleeCombo : public UGA_Ravager_Base
{
	GENERATED_BODY()

public:
	UGA_Shrewd_MeleeCombo();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
