#include "AI/BSTEval_BossAggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AI/BlackoutBossAIController.h"
#include "AI/IBossAggroProvider.h"

void FBSTEval_BossAggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.Controller->HasAuthority()) return;

	IBossAggroProvider* Provider = Cast<IBossAggroProvider>(Data.Controller->GetPawn());
	if (!Provider) return;

	APawn* BestTarget = Provider->GetHighestAggroTarget();
	Data.OutTarget    = BestTarget;
	Data.Controller->WriteTargetToBlackboard(BestTarget);
}
