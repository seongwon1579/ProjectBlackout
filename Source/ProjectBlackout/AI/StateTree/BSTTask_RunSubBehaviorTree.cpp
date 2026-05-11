#include "AI/StateTree/BSTTask_RunSubBehaviorTree.h"
#include "StateTreeExecutionContext.h"
#include "AI/BlackoutBossAIController.h"

EStateTreeRunStatus FBSTTask_RunSubBehaviorTree::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.Controller && InstanceData.SubTreeAsset)
	{
		// InstanceData.Controller->RunSubBehaviorTree(InstanceData.SubTreeAsset, InstanceData.InitialTarget);
		// return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_RunSubBehaviorTree::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// BTTask_CheckPhaseExit 가 BT를 자체 종료한 경우 StateTree에 Succeeded 반환
	// → StateTree 가 이 상태의 Transition 조건을 평가해 다음 페이즈로 전환한다
	// if (InstanceData.Controller && !InstanceData.Controller->IsSubBehaviorTreeRunning())
	// {
	// 	return EStateTreeRunStatus::Succeeded;
	// }

	return EStateTreeRunStatus::Running;
}

void FBSTTask_RunSubBehaviorTree::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.Controller)
	{
		//InstanceData.Controller->StopSubBehaviorTree();
	}
}
