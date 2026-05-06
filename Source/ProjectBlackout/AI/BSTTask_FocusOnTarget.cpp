#include "AI/BSTTask_FocusOnTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"


EStateTreeRunStatus FBSTTask_FocusOnTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_FocusOnTarget::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (Data.TargetPawn)
	{
		Data.Controller ->SetFocus(Data.TargetPawn);
	}
	else
	{
		Data.Controller->ClearFocus(EAIFocusPriority::Default);
	}
	return EStateTreeRunStatus::Running;
}

void FBSTTask_FocusOnTarget::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (Data.Controller)
	{
		Data.Controller ->ClearFocus(EAIFocusPriority::Default);
	}
}
