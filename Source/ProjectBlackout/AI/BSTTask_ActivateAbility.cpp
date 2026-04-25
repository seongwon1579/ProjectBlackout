#include "AI/BSTTask_ActivateAbility.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"

EStateTreeRunStatus FBSTTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.ASC || !InstanceData.AbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	bool bActivated = InstanceData.ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InstanceData.AbilityTag));
	
	if (bActivated)
	{
		return InstanceData.bWaitForEnd ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_ActivateAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bWaitForEnd && InstanceData.ASC)
	{
		// TODO: Ability가 끝났는지 체크
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Succeeded;
}
