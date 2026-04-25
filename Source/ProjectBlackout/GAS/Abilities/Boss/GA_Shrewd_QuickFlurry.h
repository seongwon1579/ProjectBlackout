#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GA_Shrewd_QuickFlurry.generated.h"

/**
 * 발판 페이즈 — 짧은 딜레이 후 복수 화살 연사
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_QuickFlurry : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shrewd_QuickFlurry();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	int32 ShotCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	float IntervalSeconds = 0.2f;
};
