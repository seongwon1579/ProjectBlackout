// Fill out your copyright notice in the Description page of Project Settings.

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
