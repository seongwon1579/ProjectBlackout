// ─── 구현 내역 ───────────────────────
//  - 김민영: ASC 체력 어트리뷰트로 현재 체력 비율을 매 틱 계산해 퍼블리시하는 StateTree 평가기
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_HealthRatio.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTEval_HealthRatioInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere, Category = "Output")
	float OutRatio = 1.0f;
};

USTRUCT(meta = (DisplayName = "Health Ratio Evaluator", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTEval_HealthRatio : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTEval_HealthRatioInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
