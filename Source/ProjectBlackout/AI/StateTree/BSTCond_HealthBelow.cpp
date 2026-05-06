#include "AI/StateTree/BSTCond_HealthBelow.h"
#include "StateTreeExecutionContext.h"

bool FBSTCond_HealthBelow::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.HealthRatio <= InstanceData.Ratio;
}
