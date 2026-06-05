#include "AI/StateTree/BSTEval_ShrewdAggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "AI/BlackoutAggroComponent.h"



void FBSTEval_ShrewdAggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	Data.OutTarget =nullptr;
	
	if (!Data.Controller)
	{
		return;
	}
	
	APawn* Pawn = Data.Controller->GetPawn();
	if (!Pawn)
	{
		return;
	}
	if (UBlackoutAggroComponent* Aggro = Pawn->FindComponentByClass<UBlackoutAggroComponent>())
	{
		Data.OutTarget = Aggro->GetCurrentTarget();
	}
}
