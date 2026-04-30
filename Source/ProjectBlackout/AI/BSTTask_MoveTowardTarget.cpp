#include "AI/BSTTask_MoveTowardTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "AI/BOAICalcHelper.h"

EStateTreeRunStatus FBSTTask_MoveTowardTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.TargetPawn || !Data.Controller->HasAuthority())
	{
		return EStateTreeRunStatus::Failed;
	}
	// MoveToActor 도달 거리는 작게 두고, Succeeded 트리거는 Tick의 IsWithinRange가 담당
	// (도달 판정 거리와 Tick 거리 임계값이 같으면 Path Sampling 갭으로 stuck 발생)
	Data.Controller->MoveToActor(Data.TargetPawn, 50.0f);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_MoveTowardTarget::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.TargetPawn)
	{
		return EStateTreeRunStatus::Failed;
	}
	APawn* Owner = Data.Controller->GetPawn();
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}
	return UBOAICalcHelper::IsWithinRange(Owner, Data.TargetPawn, Data.AcceptableDistance)
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}

void FBSTTask_MoveTowardTarget::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (Data.Controller)
	{
		Data.Controller->StopMovement();
	}
}
