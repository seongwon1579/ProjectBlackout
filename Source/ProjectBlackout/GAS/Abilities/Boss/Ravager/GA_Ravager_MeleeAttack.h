// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Base.h"
#include "GameplayEffect.h"
#include "GA_Ravager_MeleeAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
struct FHitResult;
struct FGameplayEventData;
class UAbilityTask_BossMeleeHitbox;

UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_MeleeAttack : public UGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_BossMeleeHitbox>> ActiveHitboxTasks;

private:
	UFUNCTION()
	void OnCollision(FGameplayEventData Payload);

	UFUNCTION()
	void OffCollision(const FHitResult& HitResult);

	UFUNCTION()
	void EndHitBox(FGameplayEventData Payload);
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitCollisionEvent;
	
};
