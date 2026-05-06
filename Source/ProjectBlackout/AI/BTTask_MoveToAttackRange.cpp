#include "AI/BTTask_MoveToAttackRange.h"
#include "AI/BTNodeHelper.h"
#include "AI/BOAICalcHelper.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/PathFollowingComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMoveToAttackRange, Log, All);

UBTTask_MoveToAttackRange::UBTTask_MoveToAttackRange()
{
	NodeName = "Move To Attack Range";
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToAttackRange::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = UBTNodeHelper::GetAIController(OwnerComp);
	if (!AI) return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB) return EBTNodeResult::Failed;

	APawn* Owner= AI->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(CurrentTargetKey.SelectedKeyName));
	if (!Owner || !Target) return EBTNodeResult::Failed;

	const float ApproachDist = BB->GetValueAsFloat(ApproachDistanceKey.SelectedKeyName);
	const float CurrentDist  = UBOAICalcHelper::GetDistance2D(Owner, Target);

	// 이미 접근 거리 이내 → 즉시 성공
	if (CurrentDist <= ApproachDist)
	{
		return EBTNodeResult::Succeeded;
	}

	// 이동 시작
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalActor(Target);
	MoveReq.SetAcceptanceRadius(ApproachDist);
	MoveReq.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = AI->MoveTo(MoveReq);

	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	if (Result.Code != EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::Failed;
	}
	
	CachedComp		= &OwnerComp;
	CachedAI        = AI;
	CachedRequestID = Result.MoveId;

	AI->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
		this, &UBTTask_MoveToAttackRange::OnMoveCompleted);

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToAttackRange::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB || !CachedAI.IsValid()) return;

	const float MaxDist = BB->GetValueAsFloat(MaxDistanceKey.SelectedKeyName);
	if (MaxDist <= 0.f) return;

	APawn* Owner = CachedAI->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(CurrentTargetKey.SelectedKeyName));
	if (!Owner || !Target) return;
	
	if (UBOAICalcHelper::GetDistance2D(Owner, Target) < MaxDist)
	{
		StopAndSucceed();
	}
}

void UBTTask_MoveToAttackRange::OnMoveCompleted(
	FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (RequestID != CachedRequestID) return;

	ClearMovement();
	FinishLatentTask(*CachedComp, EBTNodeResult::Succeeded);
	ClearPtr();
}

void UBTTask_MoveToAttackRange::StopAndSucceed()
{
	ClearMovement();
	FinishLatentTask(*CachedComp, EBTNodeResult::Succeeded);
	ClearPtr();
}

void UBTTask_MoveToAttackRange::ClearMovement()
{
	if (CachedAI.IsValid())
	{
		CachedAI->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
		CachedAI->StopMovement();
	}
}

void UBTTask_MoveToAttackRange::ClearPtr()
{
	CachedAI  = nullptr;
	CachedComp = nullptr;
}

EBTNodeResult::Type UBTTask_MoveToAttackRange::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ClearMovement();
	return EBTNodeResult::Aborted;
}

FString UBTTask_MoveToAttackRange::GetStaticDescription() const
{
	return FString::Printf(TEXT("Move To Target"));
}
