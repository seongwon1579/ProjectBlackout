// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 화살 발사 베이스 — 직선/곡사 발사 로직, 곡사 타겟 지점까지 빠른 낙하
//  - 허혁: 적중 시 플레이어 스턴 게이지/피격 단계 처리
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_Base.h"
#include "BlackoutGA_Shrewd_FireArrowBase.generated.h"

/**
 * 
 */
class ABOProjectile;
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_FireArrowBase : public UBlackoutGA_Shrewd_Base
{
	GENERATED_BODY()
	
protected:
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnFireShotEvent(FGameplayEventData Payload);
	
	
	virtual void LaunchProjectile(
		ABOProjectile* Arrow,
		const FVector& SpawnLocation,
		const FVector& TargetLocation){}
	
	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile")
	FName SpawnSocketName = TEXT("BlackoutFirePoint");

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Damage")
	float DamageMagnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Damage", meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.f;
	
};
