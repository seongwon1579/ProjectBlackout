// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 소환 미니언(Hollow) 공격 어빌리티
//  - 허혁: 적중 시 플레이어 스턴 게이지 부여
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutMinionGameplayAbility.h"
#include "BlackoutGA_Hollow_Attack.generated.h"

/**
 * 
 */
class UGameplayEffect;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Hollow_Attack : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data", meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	TSubclassOf<UGameplayEffect> Effect;
	
};
