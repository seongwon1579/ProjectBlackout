// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Shrewd 어빌리티 베이스 — 타겟 설정, 선행 로직 없는 자식 구현체 즉시 시작 흐름
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutEnemyGameplayAbility.h"
#include "BlackoutGA_Shrewd_Base.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_Base : public UBlackoutEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Shrewd_Base();

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout")
	TObjectPtr<UAnimMontage> Montage;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void SetupEventListeners() {}

	virtual void PrepareAbility() { FinishPrepare(true); }

	void FinishPrepare(bool bIsSuccess = true);

	UFUNCTION()
	void OnMontageEnded();

	void PlayMontage();

	UPROPERTY(Transient)
	TObjectPtr<APawn> CachedTarget;
};
