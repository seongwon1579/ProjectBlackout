#include "AI/BehaviorTree/Tasks/BTTask_CheckPhaseExit.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_CheckPhaseExit::UBTTask_CheckPhaseExit()
{
	NodeName = "Check Phase Exit";
}

EBTNodeResult::Type UBTTask_CheckPhaseExit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	if (BB->GetValueAsBool(PhaseExitKey.SelectedKeyName))
	{
		// 안전 지점: BT 전체를 종료하면 FBSTTask_RunSubBehaviorTree::Tick이 감지해
		// StateTree 페이즈 전환을 유도한다.
		OwnerComp.StopTree(EBTStopMode::Safe);
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}