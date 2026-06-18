// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 히트박스 베이스를 상속한 Ravager 기본 공격 어빌리티
//  - 허혁: 공격 적중 시 플레이어 스턴 게이지 부여
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "GameplayEffect.h"
#include "BlackoutGA_Ravager_HitboxAttack.h"
#include "BlackoutGA_Ravager_BasicAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
struct FHitResult;
struct FGameplayEventData;
class UAbilityTask_BossMeleeHitbox;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_BasicAttack : public UBlackoutGA_Ravager_HitboxAttack
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual TSubclassOf<UGameplayEffect> GetDamageEffect() const override;
	virtual float GetDamageMagnitude() const override;
	virtual float GetStunMagnitude() const override;
	virtual const TArray<FName>& GetHitboxComponentNames() const override;
	virtual bool HasValidSettings() const override;
	
	virtual bool ShouldClearHitboxOnHit() const override { return true; }
	
};
