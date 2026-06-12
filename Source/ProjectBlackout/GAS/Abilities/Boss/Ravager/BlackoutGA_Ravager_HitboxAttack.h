// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_HitboxAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;
class UAbilityTask_BossMeleeHitbox;
/**
 * 
 */

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_HitboxAttack : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	virtual TSubclassOf<UGameplayEffect> GetDamageEffect() const { return nullptr; }
	virtual float GetDamageMagnitude() const { return 0.f; }
	virtual float GetStunMagnitude() const { return 0.f; }
	virtual const TArray<FName>& GetHitboxComponentNames() const;
	virtual bool HasValidSettings() const override;
	
	static const TArray<FName>& GetEmptyHitboxNames();

	// 적 한 명을 맞추자마자 히트박스를 끌 것인가? (true = 단발 타격, false = 범위 내 다중/관통 타격 가능)
	virtual bool ShouldClearHitboxOnHit() const { return true; }

	void ClearHitboxTasks();

private:
	void CacheHitboxComponents();

	UFUNCTION()
	void HandleHitboxEnable(FGameplayEventData Payload);

	UFUNCTION()
	void ApplyHitboxDamage(const FHitResult& HitResult);

	UFUNCTION()
	void HandleHitboxDisable(FGameplayEventData Payload);

protected:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPrimitiveComponent>> CachedHitboxComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_BossMeleeHitbox>> ActiveHitboxTasks;

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> DamagedActorsThisWindow;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitCollisionOnEvent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitCollisionOffEvent;
};
