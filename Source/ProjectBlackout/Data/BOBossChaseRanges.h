// ─── 구현 내역 ───────────────────────
//  - 조성원: 보스 추적 범위 구조체 정의 — 추적 시작/종료 거리·공격 사거리·사거리 분산(Ravager 종속을 Boss 공용으로 분리)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BOBossChaseRanges.generated.h"

USTRUCT(BlueprintType)           
struct FBossChaseRanges            
{
	GENERATED_BODY()              

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ChaseStartRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ChaseEndRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackRangeVariance = 0.f;
};