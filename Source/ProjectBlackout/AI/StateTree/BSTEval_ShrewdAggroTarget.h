// ─── 구현 내역 ───────────────────────
//  - 조성원: 컨트롤러의 어그로 타겟을 StateTree로 전달하는 평가기 (보스/잡몹 어그로 분리)
//  - 최승현: Shrewd 비행 보스 연동 및 어그로 평가 안정성/정합성 보강
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_ShrewdAggroTarget.generated.h"

class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTEval_AggroTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<APawn> OutTarget;
};

USTRUCT(meta = (DisplayName = "Aggro Target Evaluator", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTEval_ShrewdAggroTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTEval_AggroTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
