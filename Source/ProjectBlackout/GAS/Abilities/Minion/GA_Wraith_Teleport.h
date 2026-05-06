// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "GA_Wraith_Teleport.generated.h"

/**
 * Wraith 점멸.
 * Fire 직후 또는 시야 차단 시 EQS 결과 위치로 즉시 이동. 비행 중 무적.
 */
UCLASS()
class PROJECTBLACKOUT_API
	UGA_Wraith_Teleport : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Wraith_Teleport();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo
	                             ActivationInfo,
	                             const FGameplayEventData*
	                             TriggerEventData) override;
};
