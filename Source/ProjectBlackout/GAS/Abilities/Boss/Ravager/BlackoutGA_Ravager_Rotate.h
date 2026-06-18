// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 타겟 방향 회전(Rotate) 어빌리티
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_Rotate.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Rotate : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

public:
	virtual void PreActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual FGameplayTag SelectMontageTag(
		const FGameplayEventData* TriggerEventData) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To90Threshold = 25.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To135Threshold = 90.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	float To180Threshold = 135.f;
	
};
