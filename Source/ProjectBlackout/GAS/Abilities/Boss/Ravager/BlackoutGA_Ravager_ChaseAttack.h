// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutGA_Ravager_BasicAttack.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_HitboxAttack.h"
#include "BlackoutGA_Ravager_ChaseAttack.generated.h"

class UAbilityTask_TickerTask;
/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_ChaseAttack : public UBlackoutGA_Ravager_BasicAttack
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	UFUNCTION()
	void OnMoveStart(FGameplayEventData Payload);

	UFUNCTION()
	void OnMoveEnd(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMoveTick(float DeltaTime);
	
	void StopMove();

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_TickerTask> MoveTask;

	TWeakObjectPtr<const APawn> MoveTarget;
	FVector MoveStartLocation = FVector::ZeroVector;
	FVector GoalLocSmoothed   = FVector::ZeroVector;
	float MoveDuration = 0.f;
	float MoveElapsed  = 0.f;
	bool  bMoving      = false;
};
