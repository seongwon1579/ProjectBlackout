// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 소환 미니언(Hollow) 스폰 어빌리티
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutMinionGameplayAbility.h"
#include "BlackoutGA_Hollow_Spawn.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Hollow_Spawn : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()
 
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data")
	TObjectPtr<UAnimMontage> SpawnMontage;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnMontageEnded();
};
