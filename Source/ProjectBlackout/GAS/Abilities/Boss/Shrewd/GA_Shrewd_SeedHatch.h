#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "GA_Shrewd_SeedHatch.generated.h"

/**
 * 기획 보류 — 씨앗 부화 기믹
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_SeedHatch : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

public:
	UGA_Shrewd_SeedHatch();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	int32 SeedCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float InvulnerableDuration = 5.0f;
};
