// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "GameplayEffect.h"
#include "GA_BossMeleeAttack.generated.h"

struct FHitResult;
struct FGameplayEventData;
class UAbilityTask_BossMeleeHitbox;

/**
 * 보스 근접 공격 베이스 어빌리티.
 * 모션워핑 + 몽타주 + sweep 판정 + 데미지 적용 공통 로직을 담당한다.
 * 대부분의 공격은 BP 서브클래스에서 프로퍼티만 설정해서 사용한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_BossMeleeAttack : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BossMeleeAttack();

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Montage;

	/** 히트 시 적용할 데미지 GE */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 활성화할 히트박스 컴포넌트 이름 목록 (보스 BP에서 추가한 컴포넌트 이름) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<FName> HitboxComponentNames;

	UPROPERTY(EditDefaultsOnly, Category = "MotionWarping")
	FName WarpTargetName = TEXT("MW_Target");
	
	UPROPERTY(EditDefaultsOnly, Category= "Ravager" , meta= (ClampMin="0.0"))
	float DamageMagnitude = 10.0f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	
private:
	void SetupMotionWarp(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData);
	void PlayAttackMontage();
	void WaitForCollisionEvent();

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnCollision(FGameplayEventData Payload);

	UFUNCTION()
	void OffCollision(const FHitResult& HitResult);

	UFUNCTION()
	void EndHitBox(FGameplayEventData Payload);

	TArray<TObjectPtr<UAbilityTask_BossMeleeHitbox>> ActiveHitboxTasks;
};
