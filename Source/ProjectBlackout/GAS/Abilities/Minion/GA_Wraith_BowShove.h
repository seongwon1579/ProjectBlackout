// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "Templates/SubclassOf.h"
#include "GA_Wraith_BowShove.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UAbilityTask_BossMeleeSweep;
struct FGameplayEventData;
struct FHitResult;

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
	
	UFUNCTION()
	void OnSweepStartEvent(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnSweepEndEvent(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMeleeSweepHit(const FHitResult& HitResult);
	
	UFUNCTION()
	void OnMontageEnded();
	
	UPROPERTY(EditDefaultsOnly,Category="Wraith")
	TObjectPtr<UAnimMontage> BowShoveAnimMontage;
	
	UPROPERTY(EditDefaultsOnly,Category="Wraith")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Wraith" , meta=(ClampMin="0.0"))
	float DamageMagnitude =12.0f;
	
	/** 활대 시작 소켓 */
	UPROPERTY(EditDefaultsOnly, Category="Wraith|Sweep")
	FName StartSocketName = TEXT("BowShove_Start");
	
	/** 활대 끝 소켓 */
	UPROPERTY(EditDefaultsOnly, Category="Wraith|Sweep")
	FName EndSocketName = TEXT("BowShove_End");
	
	/** Sweep Sphere 반지름 */
	UPROPERTY(EditDefaultsOnly, Category="Wraith|Sweep" , meta=(ClampMin="0.0"))
	float SweepRadius = 30.0f;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_BossMeleeSweep> ActiveSweepTask;
	
};
