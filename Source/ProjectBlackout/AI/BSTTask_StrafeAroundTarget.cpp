#include "AI/BSTTask_StrafeAroundTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "AI/BOAICalcHelper.h"

EStateTreeRunStatus FBSTTask_StrafeAroundTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.TargetPawn || !Data.Controller->HasAuthority())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const APawn* Owner = Data.Controller->GetPawn();
	const UWorld* World = Data.Controller->GetWorld();
	if (!Owner || !World)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	Data.CachedRadius = UBOAICalcHelper::GetDistance2D(Owner, Data.TargetPawn);
	Data.bStrafeRight = FMath::RandBool();
	Data.NextSwitchTime = World->GetTimeSeconds();
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_StrafeAroundTarget::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data =Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.TargetPawn)
	{
		return EStateTreeRunStatus::Failed;
	}
	const APawn* Owner = Data.Controller->GetPawn();
	const UWorld* World = Data.Controller->GetWorld();
	if (!Owner || !World)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const float Now =World->GetTimeSeconds();
	if (Now < Data.NextSwitchTime)
	{
		return EStateTreeRunStatus::Running;
	}
	
	Data.bStrafeRight = !Data.bStrafeRight;
	Data.NextSwitchTime =Now+ Data.StrafeInterval;
	
	const FVector TargetLocation = Data.TargetPawn->GetActorLocation();
	const FVector FromTarget = (Owner -> GetActorLocation() - TargetLocation).GetSafeNormal2D();
	const float Angle = Data.bStrafeRight ? Data.StrafeAngleDeg: -Data.StrafeAngleDeg;
	const FVector Goal = TargetLocation + FromTarget.RotateAngleAxis(Angle, FVector::UpVector)*Data.CachedRadius;
	
	Data.Controller->MoveToLocation(Goal);
	return EStateTreeRunStatus::Running;
}

void FBSTTask_StrafeAroundTarget::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (Data.Controller) {Data.Controller->StopMovement(); }
}
