#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Shrewd_Lunge.generated.h"

/**
 * 지면 페이즈 — 장거리 강습 도약 (Motion Warping)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_Lunge : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shrewd_Lunge();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float LungeDistance = 1500.0f;
};
