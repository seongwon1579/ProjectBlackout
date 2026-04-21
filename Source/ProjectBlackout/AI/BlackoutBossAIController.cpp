#include "BlackoutBossAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

ABlackoutBossAIController::ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SubBehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("SubBehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ABlackoutBossAIController::InitStateTreeContext()
{
	Super::InitStateTreeContext();
	// Bind ASC, Pawn, BBComp handles to StateTree context for FBSTEval_AggroTarget, etc.
}

void ABlackoutBossAIController::RunSubBehaviorTree(UBehaviorTree* SubTree)
{
	if (!SubTree || !SubBehaviorTreeComp || !BlackboardComp)
	{
		return;
	}

	// 이미 실행 중인 트리가 있으면 먼저 정지
	if (SubBehaviorTreeComp->IsRunning())
	{
		StopSubBehaviorTree();
	}

	// Blackboard 초기화 (SubTree에 에셋이 지정된 경우)
	if (SubTree->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*SubTree->BlackboardAsset);
	}

	SubBehaviorTreeComp->StartTree(*SubTree);
}

void ABlackoutBossAIController::StopSubBehaviorTree()
{
	if (SubBehaviorTreeComp)
	{
		SubBehaviorTreeComp->StopTree(EBTStopMode::Safe);
	}
}

void ABlackoutBossAIController::WriteTargetToBlackboard(APawn* TargetPawn)
{
	if (BlackboardComp)
	{
		static const FName TargetKeyName(TEXT("BB_CurrentTarget"));
		BlackboardComp->SetValueAsObject(TargetKeyName, TargetPawn);
	}
}
