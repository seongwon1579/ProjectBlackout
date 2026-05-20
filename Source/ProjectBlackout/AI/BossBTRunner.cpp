#include "AI/BossBTRunner.h"
#include "AI/BlackoutBossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Pawn.h"

void UBossBTRunner::Initialize(ABlackoutBossAIController* InController,
                               const TMap<EBossPhase, TObjectPtr<UBehaviorTree>>& InTrees)
{
	OwnerController = InController;
	PhaseBehaviorTrees = InTrees;
}

void UBossBTRunner::RunPhaseBT(EBossPhase NewPhase)
{
	if (!OwnerController || NewPhase == EBossPhase::None) return;

	TObjectPtr<UBehaviorTree>* Tree = PhaseBehaviorTrees.Find(NewPhase);

	if (!Tree || !*Tree) return;

	UE_LOG(LogTemp, Log,
	       TEXT("BTRunner: Running BT for phase %d → %d"),
	       (int32)CurrentPhase, (int32)NewPhase);
	
	OwnerController->RunBehaviorTree(*Tree);
	
	CurrentPhase = NewPhase;
}

void UBossBTRunner::StopBT()
{
	if (!OwnerController) return;
	
	if (UBrainComponent* Brain = OwnerController->GetBrainComponent())
	{
		Brain->StopLogic(TEXT("BTRunner::StopBT"));
	}
	
	CurrentPhase = EBossPhase::None;
}
