#include "AI/BehaviorTree/BTNodeHelper.h"
#include "AbilitySystemGlobals.h"


APawn* UBTNodeHelper::GetAIPawn(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	return AI ? AI->GetPawn() : nullptr;
}

AActor* UBTNodeHelper::GetActorFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? Cast<AActor>(BB->GetValueAsObject(Key.SelectedKeyName)) : nullptr;
}

TOptional<float> UBTNodeHelper::GetFloatFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? BB->GetValueAsFloat(Key.SelectedKeyName) : TOptional<float>();
}

UAbilitySystemComponent* UBTNodeHelper::GetAbilitySystemComponent(UBehaviorTreeComponent& OwnerComp)
{
	APawn* Pawn = GetAIPawn(OwnerComp);
	return Pawn ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn) : nullptr;
}
