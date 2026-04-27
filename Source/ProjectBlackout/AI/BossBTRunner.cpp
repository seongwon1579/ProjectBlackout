#include "AI/BossBTRunner.h"
#include "AI/BlackoutBossAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Pawn.h"

const FName UBossBTRunner::TargetKeyName    = TEXT("BB_CurrentTarget");
const FName UBossBTRunner::PhaseExitKeyName = TEXT("BB_RequestPhaseExit");

void UBossBTRunner::Initialize(ABlackoutBossAIController* InOwner,
                                UBehaviorTreeComponent*    InBTComp,
                                UBlackboardComponent*      InBBComp)
{
	OwnerController = InOwner;
	BTComp          = InBTComp;
	BBComp          = InBBComp;
}

void UBossBTRunner::RunBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget)
{
	//if (!HasAuthority() || !SubTree || !BTComp.IsValid() || !BBComp.IsValid()) return;
	if (!SubTree || !BTComp.IsValid() || !BBComp.IsValid()) return;

	if (BTComp->IsRunning())
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}

	if (SubTree->BlackboardAsset)
	{
		BBComp->InitializeBlackboard(*SubTree->BlackboardAsset);
	}

	BTComp->StartTree(*SubTree);
	
	UE_LOG(LogTemp, Warning, TEXT("RunBehaviorTree"));
	
	// APawn* Target = InitialTarget ? InitialTarget : CachedTarget.Get();
	// if (Target)
	// {
	// 	CachedTarget = Target;
	// 	BBComp->SetValueAsObject(TargetKeyName, Target);
	// }
}

void UBossBTRunner::Stop()
{
	if (BTComp.IsValid() && BTComp->IsRunning())
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}
}

bool UBossBTRunner::IsRunning() const
{
	return BTComp.IsValid() && BTComp->IsRunning();
}

void UBossBTRunner::WriteTargetToBlackboard(APawn* TargetPawn)
{
	if (!HasAuthority() || !BBComp.IsValid()) return;

	CheckingActor = TargetPawn;
	CachedTarget = TargetPawn;
	BBComp->SetValueAsObject(TargetKeyName, TargetPawn);
}

void UBossBTRunner::RequestPhaseExit()
{
	if (!HasAuthority() || !BBComp.IsValid()) return;
	BBComp->SetValueAsBool(PhaseExitKeyName, true);
}

bool UBossBTRunner::HasAuthority() const
{
	return OwnerController.IsValid() && OwnerController->HasAuthority();
}