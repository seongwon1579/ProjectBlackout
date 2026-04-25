#include "AI/BTDecorator_CompareDistance.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CompareDistance::UBTDecorator_CompareDistance()
{
	NodeName = "Compare Distance";
}

bool UBTDecorator_CompareDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	const float Distance = BB->GetValueAsFloat(DistanceKey.SelectedKeyName);

	if (Distance < MinDistance) return false;
	if (MaxDistance > 0.f && Distance > MaxDistance) return false;

	return true;
}

FString UBTDecorator_CompareDistance::GetStaticDescription() const
{
	return FString::Printf(TEXT("Distance: %.0f ~ %s"),
		MinDistance,
		MaxDistance > 0.f ? *FString::Printf(TEXT("%.0f"), MaxDistance) : TEXT("∞"));
}