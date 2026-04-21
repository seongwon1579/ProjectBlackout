#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_Shrewd_ExplosiveArrow.generated.h"

/**
 * 발판 페이즈 — 폭발 속성 단발 투사체
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_ExplosiveArrow : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shrewd_ExplosiveArrow();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float SplashRadius = 500.0f;
};
