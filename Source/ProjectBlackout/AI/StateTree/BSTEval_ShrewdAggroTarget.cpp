#include "AI/StateTree/BSTEval_ShrewdAggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "BlackoutShrewdAIController.h"



void FBSTEval_ShrewdAggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!Data.Controller)
	{
		Data.OutTarget = nullptr;
		return;
	}
	
	if (ABlackoutShrewdAIController* Controller = Cast<ABlackoutShrewdAIController>(Data.Controller))
	{
		Data.OutTarget = Controller->GetCurrentAggroTarget();
	}
}
