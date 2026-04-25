#include "AI/BSTTask_Charge.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BlackoutEnemyCharacter.h"

EStateTreeRunStatus FBSTTask_Charge::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (InstanceData.OwnerCharacter)
	{
		// TODO: OwnerCharacter의 PerformCharge() 호출
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}
