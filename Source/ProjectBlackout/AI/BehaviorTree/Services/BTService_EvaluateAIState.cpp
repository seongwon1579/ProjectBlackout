#include "AI/BehaviorTree/Services/BTService_EvaluateAIState.h"

#include "AI/BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_EvaluateAIState::UBTService_EvaluateAIState()
{
	NodeName = TEXT("Evaluate AI State");
	Interval = 0.1f;
	RandomDeviation = 0.05f;
}

void UBTService_EvaluateAIState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB) return;

	const float Distance = BB->GetValueAsFloat(CurrentDistanceKey.SelectedKeyName);
	const float Approach = BB->GetValueAsFloat(ApproachDistanceKey.SelectedKeyName);
	//
	// const uint8 NewState = (Distance > Approach + Approach * Threshold)
	// 	? OutOfRangeStateIndex
	// 	: ChaseStateIndex;
	//
	// UE_LOG(LogTemp, Warning, TEXT("New State Index: %i"), NewState);
	//
	// BB->SetValueAsEnum(AIStateKey.SelectedKeyName, NewState);
	
	BB->SetValueAsBool(bChaseKey.SelectedKeyName, Distance < Approach + Approach * Threshold);
	
}
