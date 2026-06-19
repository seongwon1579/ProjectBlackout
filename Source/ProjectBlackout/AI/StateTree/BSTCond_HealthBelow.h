// ─── 구현 내역 ───────────────────────
//  - 김민영: 퍼블리시된 체력 비율이 기준치 미만인지 판정하는 StateTree 조건
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "BSTCond_HealthBelow.generated.h"

USTRUCT()
struct PROJECTBLACKOUT_API FBSTCond_HealthBelowInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	float HealthRatio = 1.0f; // FBSTEval_HealthRatio 가 퍼블리시한 값과 바인딩

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Ratio = 0.5f;
};

USTRUCT(meta = (DisplayName = "Health Below", Category = "Blackout|Boss"))
struct PROJECTBLACKOUT_API FBSTCond_HealthBelow : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTCond_HealthBelowInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
