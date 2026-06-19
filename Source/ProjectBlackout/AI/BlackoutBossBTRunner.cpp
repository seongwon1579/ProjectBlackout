#include "AI/BlackoutBossBTRunner.h"
#include "AI/BlackoutBossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Pawn.h"

void UBlackoutBossBTRunner::Initialize(ABlackoutBossAIController* InController,
                               const TMap<EBOBossPhase, TObjectPtr<UBehaviorTree>>& InTrees)
{
	OwnerController = InController;
	PhaseBehaviorTrees = InTrees;
}

void UBlackoutBossBTRunner::RunPhaseBT(EBOBossPhase NewPhase)
{
	if (!OwnerController || NewPhase == EBOBossPhase::None) return;
	
	TObjectPtr<UBehaviorTree>* Tree = PhaseBehaviorTrees.Find(NewPhase);

	if (!Tree || !*Tree) return;

	UE_LOG(LogTemp, Log,
	       TEXT("BTRunner: Running BT for phase %d → %d"),
	       (int32)CurrentPhase, (int32)NewPhase);
	
	OwnerController->RunBehaviorTree(*Tree);
	
	CurrentPhase = NewPhase;
}

void UBlackoutBossBTRunner::StopBT()
{
	if (!OwnerController) return;
	
	if (UBrainComponent* Brain = OwnerController->GetBrainComponent())
	{
		Brain->StopLogic(TEXT("BTRunner::StopBT"));
	}
	
	CurrentPhase = EBOBossPhase::None;
}
