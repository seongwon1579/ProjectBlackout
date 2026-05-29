#pragma once

#include "CoreMinimal.h"
#include "BlackoutGA_Shrewd_Base.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "GA_Shrewd_ExplosiveArrow.generated.h"

/**
 * 발판 페이즈 — 폭발 속성 단발 투사체
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_ExplosiveArrow : public UBlackoutGA_Shrewd_Base
{
	GENERATED_BODY()

public:
	UGA_Shrewd_ExplosiveArrow();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float SplashRadius = 500.0f;
};
