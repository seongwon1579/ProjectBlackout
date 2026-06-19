// ─── 구현 내역 ───────────────────────
//  - 김민영: 근접 밀치기(Bow Shove) StateTree Task 정의
//  - 최승현: GA 태그 트리거로 밀치기 실행/완료 판정 구현 (거리 체크는 Cond가 담당)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_BowShove.generated.h"

class APawn;

/**
 * Wraith 근접 밀치기 StateTree Task.
 * 거리 체크는 ST Cond(BSTCond_TargetWithinRange)가 책임. 본 Task는 GA 트리거만.
 *
 * 흐름:
 *   EnterState → ASC->TryActivateAbilitiesByTag(Ability.Wraith.BowShove)
 *   Tick       → GA Active 체크 → Running / Succeeded
 */
USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_BowShoveInstanceData
{
	GENERATED_BODY()

	/** BowShove를 수행할 적 폰. */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> OwnerPawn;
};

USTRUCT(meta = (DisplayName = "Bow Shove", Category = "Blackout|Minion"))
struct PROJECTBLACKOUT_API FBSTTask_BowShove : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_BowShoveInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
