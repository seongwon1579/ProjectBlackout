#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Shrewd_LoSTeleport.generated.h"

/**
 * LoS 차단 시 즉시 점멸 후 근접 콤보 연계
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_LoSTeleport : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shrewd_LoSTeleport();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
