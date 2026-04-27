#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_RunSubBehaviorTree.generated.h"

class ABlackoutBossAIController;
class UBehaviorTree;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_RunSubBehaviorTreeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABlackoutBossAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UBehaviorTree> SubTreeAsset;

	/** FBSTEval_AggroTarget::OutTarget 에 바인딩. BT 시작 시 BB에 주입된다. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<APawn> InitialTarget;
};

USTRUCT(meta = (DisplayName = "Run Sub Behavior Tree", Category = "Blackout|Boss"))
struct PROJECTBLACKOUT_API FBSTTask_RunSubBehaviorTree : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_RunSubBehaviorTreeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick      (FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void                ExitState (FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
