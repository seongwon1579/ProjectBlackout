// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "GA_Wraith_BowShove.generated.h"

/**
 * Wraith 근접공격
 * 근접 감지 시 Fire 캔슬 후 발동, 임팩트 시 타겟 knockback + 짧은 경직
 */
UCLASS()
class PROJECTBLACKOUT_API
	UGA_Wraith_BowShove : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Wraith_BowShove();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo
	                             ActivationInfo,
	                             const FGameplayEventData*
	                             TriggerEventData) override;
};
