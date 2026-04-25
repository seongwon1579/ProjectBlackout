#include "AI/BSTTask_Teleport.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BlackoutEnemyCharacter.h"

EStateTreeRunStatus FBSTTask_Teleport::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwnerCharacter)
	{
		// TODO: 점멸 로직 실행 (Wraith의 TeleportOutOfSight 등 호출)
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}
