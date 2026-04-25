#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BOBossData.generated.h"

/**
 * 보스(Shrewd / Corrupted Ravager) 페이즈·패턴·어그로 튜닝 데이터.
 * ABlackoutBossCharacter, UBlackoutAggroComponent에서 참조.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOBossData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ── 페이즈 ────────────────────────────────────────────────────────────────

	/**
	 * 체력 비율(0~1) 기준 페이즈 컷라인. 오름차순 정렬.
	 * 기본값: [0.6, 0.3] → Phase A(100~60%), B(60~30%), C(30~0%)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Phase")
	TArray<float> PhaseHealthCutlines = { 0.6f, 0.3f };

	// ── 히트박스 배율 ─────────────────────────────────────────────────────────

	/**
	 * 부위 태그별 피해 배율.
	 * Body.WeakSpot → 1.5 / Body.ArmoredLimb → 0.5 (TDD §5.2)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Hitbox")
	TMap<FGameplayTag, float> HitPartMultipliers;

	// ── 패턴 데미지 ───────────────────────────────────────────────────────────

	/**
	 * 패턴 어빌리티 태그별 기본 데미지 값.
	 * Key: GA 태그 (e.g. GA.Ravager.DoubleSwipe)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Abilities")
	TMap<FGameplayTag, float> AbilityDamageMap;

	// ── 어그로 튜닝 (TDD §6.1) ────────────────────────────────────────────────

	/** 타겟 전환 최소 쿨다운 (초). 핑퐁 방지. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Aggro", meta = (ClampMin = 0.f))
	float AggroSwitchCooldown = 5.0f;

	/** 누적 피해 1순위 우선 임계값 비율. 2위와 격차가 이 값 미만이면 2순위로 넘어감. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Aggro", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float AggroDamageThreshold = 0.15f;

	/** 누적 피해량 초당 감쇠율. 장기전에서 초반 피해 고착 방지. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Aggro", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float AggroDecayRate = 0.02f;
};
