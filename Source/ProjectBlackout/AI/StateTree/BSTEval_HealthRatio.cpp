#include "AI/StateTree/BSTEval_HealthRatio.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"

void FBSTEval_HealthRatio::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.ASC)
	{
		// TODO: 실제 UBlackoutBaseAttributeSet 에서 Health / MaxHealth 를 가져와 비율 계산
	}
}
