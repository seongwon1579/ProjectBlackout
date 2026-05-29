#include "AI/StateTree/BSTEval_AggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"

void FBSTEval_AggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.Controller->HasAuthority()) return;

	APawn* OwnerPawn = Data.Controller->GetPawn();
	if (!OwnerPawn) return;

	UAIPerceptionComponent* PerceptionComp = Data.Controller->GetAIPerceptionComponent();
	if (!PerceptionComp) return;

	TArray<AActor*> Perceived;
	PerceptionComp->GetCurrentlyPerceivedActors(nullptr, Perceived);

	APawn* BestTarget = nullptr;
	float BestDistSq  = MAX_FLT;

	for (AActor* Actor : Perceived)
	{
		ABlackoutPlayerCharacter* Player = Cast<ABlackoutPlayerCharacter>(Actor);
		if (!Player) continue;

		const float DistSq = FVector::DistSquared(
			OwnerPawn->GetActorLocation(), Player->GetActorLocation());

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Player;
		}
	}

	Data.OutTarget = BestTarget;
}
