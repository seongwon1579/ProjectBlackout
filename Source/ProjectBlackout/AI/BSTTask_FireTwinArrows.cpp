#include "AI/BSTTask_FireTwinArrows.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "Combat/Weapons/BOProjectile.h"

EStateTreeRunStatus FBSTTask_FireTwinArrows::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwnerCharacter)
	{
		// TODO: 2연사 로직 시작
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_FireTwinArrows::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// TODO: 시간 간격 확인 후 발사 및 완료 판정
	return EStateTreeRunStatus::Succeeded;
}
