#include "AI/BTService_LineOfSightCheck.h"
#include "AI/BTNodeHelper.h"

UBTService_LineOfSightCheck::UBTService_LineOfSightCheck()
{
	NodeName = "Line of Sight Check";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_LineOfSightCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AI = UBTNodeHelper::GetAIController(OwnerComp);
	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!AI || !BB) return;

	AActor* Target = UBTNodeHelper::GetActorFromBB(OwnerComp, TargetKey);
	const bool bHasLoS = Target ? AI->LineOfSightTo(Target) : false;

	BB->SetValueAsBool(OutHasLineOfSightKey.SelectedKeyName, bHasLoS);
}
