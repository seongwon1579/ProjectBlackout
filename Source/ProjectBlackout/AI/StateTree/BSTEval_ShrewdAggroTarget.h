#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_AggroTarget.generated.h"

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
struct PROJECTBLACKOUT_API FBSTEval_AggroTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTEval_AggroTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
