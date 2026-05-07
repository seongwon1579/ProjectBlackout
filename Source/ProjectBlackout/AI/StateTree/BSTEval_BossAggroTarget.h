#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_BossAggroTarget.generated.h"

class ABlackoutBossAIController;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTEval_BossAggroTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABlackoutBossAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<APawn> OutTarget;
};

USTRUCT(meta = (DisplayName = "Boss Aggro Target Evaluator", Category = "Blackout|Boss"))
struct PROJECTBLACKOUT_API FBSTEval_BossAggroTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTEval_BossAggroTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
