#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_WraithAggroTarget.generated.h"


class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTEval_WraithAggroTargetInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere , Category="Context")
	TObjectPtr<AAIController> Controller;
	
	// 거리 가중치 (0~1) - 가까울수록 점수 ↑
	UPROPERTY(EditAnywhere , Category="Tuning|Weights", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DistanceWeight= 1.0f;
	
	// 거리 정규화 기준 - 이 거리 안에서 점수 1~0 밖이면 0
	UPROPERTY(EditAnywhere, Category="Tuning|Weights" ,meta=(ClampMin="100.0"))
	float MaxRange = 99999.0f;
	
	UPROPERTY(EditAnywhere,Category="Output")
	TObjectPtr<APawn> OutTarget;
};

/**
 * Wraith 어그로 평가
 * 현재 거리 가중치만 적용 , 페어 분산 / HP 가중 / 위협 누적 확장은 예정
 */
USTRUCT(meta=(DisplayName="Wraith Aggro Target Evaluator",Category="Blackout|Wraith"))
struct PROJECTBLACKOUT_API FBSTEval_WraithAggroTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTEval_WraithAggroTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
};


