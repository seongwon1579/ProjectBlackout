#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "BSTEval_AggroTarget.generated.h"

class UAbilitySystemComponent;
class ABlackoutBossAIController;
class UBOBossData;
class APlayerState;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTEval_AggroTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UAbilitySystemComponent> OwnerASC;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABlackoutBossAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UBOBossData> BossData;

	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<APawn> OutTarget;

	TMap<TWeakObjectPtr<APlayerState>, float> DamageAccumulator;
	TWeakObjectPtr<APlayerState> CurrentTargetPS;
	float LastSwitchTime = 0.0f;
};

USTRUCT(meta = (DisplayName = "Aggro Target Evaluator", Category = "Blackout|Boss"))
struct PROJECTBLACKOUT_API FBSTEval_AggroTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTEval_AggroTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
