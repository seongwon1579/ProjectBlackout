// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_HitboxAttack.h"
#include "BlackoutGA_Ravager_Charge.generated.h"


class USceneComponent;
class UGameplayEffect;
class UAbilityTask_TickerTask;
/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Charge : public UBlackoutGA_Ravager_HitboxAttack
{
	GENERATED_BODY()

protected:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	
	virtual void SetupEventListeners() override;

	virtual TSubclassOf<UGameplayEffect> GetDamageEffect() const override;
	virtual float GetDamageMagnitude() const override;
	virtual const TArray<FName>& GetHitboxComponentNames() const override;
	virtual bool HasValidSettings() const override;

	virtual bool ShouldClearHitboxOnHit() const override { return false; }
	
	UFUNCTION()
	void HandleLoopStart(FGameplayEventData Payload);
	
	UFUNCTION()
	void TickChargeMovement(float DeltaTime);
	
	void TickCheckDistance();
	void GoToEndSection();

private:
	UPROPERTY(transient)
	TWeakObjectPtr<USceneComponent> CachedTargetComponent;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_TickerTask> ChargeTickTask;
	
	FTimerHandle CheckTimerHandle;
	
	float ChargeStartTime = 0.f;
	
	float CachedOriginalMaxWalkSpeed = 0.f;
	
	float bEndingTriggered = false;
};
