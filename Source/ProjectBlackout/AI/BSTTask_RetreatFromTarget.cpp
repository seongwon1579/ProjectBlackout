#include "AI/BSTTask_RetreatFromTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "AI/BOAICalcHelper.h"

EStateTreeRunStatus FBSTTask_RetreatFromTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.TargetPawn || !Data.Controller->HasAuthority())
	{
		return EStateTreeRunStatus::Failed;
	}
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_RetreatFromTarget::Tick(
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
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector ToTarget = Data.TargetPawn->GetActorLocation()-OwnerLocation;
	const float Distance = UBOAICalcHelper::GetDistance2D(Owner, Data.TargetPawn);
	
	if (Distance>= Data.SafeDistance)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	
	const FVector RetreatDir = -ToTarget.GetSafeNormal2D();
	const FVector RetreatGoal = OwnerLocation + RetreatDir* (Data.SafeDistance - Distance + Data.OvershootBuffer);
	Data.Controller->MoveToLocation(RetreatGoal);
	return EStateTreeRunStatus::Running;
}

void FBSTTask_RetreatFromTarget::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (Data.Controller)
	{
		Data.Controller->StopMovement();
	}
}
