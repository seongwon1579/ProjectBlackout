#include "AI/BSTEval_AggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"

void FBSTEval_AggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller) return;

	UAIPerceptionComponent* PerceptionComp = Data.Controller->GetAIPerceptionComponent();
	if (!PerceptionComp) return;

	APawn* OwnerPawn = Data.Controller->GetPawn();

	TArray<AActor*> Perceived;
	PerceptionComp->GetCurrentlyPerceivedActors(nullptr, Perceived);

	APawn* BestTarget = nullptr;
	float BestDistSq  = MAX_FLT;

	for (AActor* Actor : Perceived)
	{
		APawn* Pawn = Cast<APawn>(Actor);
		if (!Pawn || Pawn == OwnerPawn) continue;

		const float DistSq = FVector::DistSquared(
			OwnerPawn->GetActorLocation(), Pawn->GetActorLocation());

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Pawn;
		}
	}

	Data.OutTarget = BestTarget;
}
