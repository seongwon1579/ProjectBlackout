// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: Wraith 쌍화살 사격 GA — Projectile 발사, Bowshot 몽타주 + AimDelay 조준 선딜, AnimNotify 통합, ASC 쿨다운 + 타겟 응시 Task
//  - 허혁: 적중 시 플레이어 스턴 게이지/피격 단계 처리
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "Templates/SubclassOf.h"
#include "GA_Wraith_FireTwinArrows.generated.h"

class ABOProjectile;
class UAnimMontage;
struct FGameplayEventData;

/**
 * Wraith 2연발 화살.
 * AnimNotify가 SendGameplayEvent → GA가 WaitGameplayEvent로 격리 수신.
 * Notify 두 개가 같은 태그 발사 → OnFireShotEvent 두 번 호출 → 화살 두 발.
 * 사거리 도달 + 시야 확보 시 발동. 발사 완료 직후 Teleport 연계.
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Wraith_FireTwinArrows : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Wraith_FireTwinArrows();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnFireShotEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	void FireOneArrow();

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith", meta = (ClampMin = "0.0"))
	float DamageMagnitude = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith", meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TObjectPtr<UAnimMontage> BowshotMontage;
};
