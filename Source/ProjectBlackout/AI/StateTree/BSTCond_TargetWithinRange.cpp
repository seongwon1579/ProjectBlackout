#include "AI/StateTree/BSTCond_TargetWithinRange.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"
#include "AI/BOAICalcHelper.h"

bool FBSTCond_TargetWithinRange::TestCondition(
	FStateTreeExecutionContext& Context) const
{
	
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!Data.OwnerPawn|| !Data.TargetPawn)
	{
		return false;
	}
	
	const float Distance = Data.bUse2DDistance
		? UBOAICalcHelper::GetDistance2D(Data.OwnerPawn, Data.TargetPawn)
		: FVector::Dist(Data.OwnerPawn->GetActorLocation(), Data.TargetPawn->GetActorLocation());
	
	return Distance >= Data.MinRange && Distance <= Data.MaxRange;
}
