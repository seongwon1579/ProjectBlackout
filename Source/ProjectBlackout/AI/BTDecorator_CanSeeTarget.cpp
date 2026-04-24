#include "AI/BTDecorator_CanSeeTarget.h"
#include "AI/BTNodeHelper.h"

UBTDecorator_CanSeeTarget::UBTDecorator_CanSeeTarget()
{
	NodeName = "Can See Target";
}

bool UBTDecorator_CanSeeTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AI = UBTNodeHelper::GetAIController(OwnerComp);
	AActor* Target = UBTNodeHelper::GetActorFromBB(OwnerComp, TargetKey);
	if (!AI || !Target) return false;

	return AI->LineOfSightTo(Target);
}