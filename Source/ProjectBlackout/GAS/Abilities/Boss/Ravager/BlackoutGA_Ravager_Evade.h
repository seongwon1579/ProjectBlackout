// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 회피(Evade) 어빌리티 — Template 상속 구조로 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_Evade.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Evade : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()
	
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
