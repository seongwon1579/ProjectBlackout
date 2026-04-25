#include "AI/BTNodeHelper.h"
#include "AbilitySystemGlobals.h"

AAIController* UBTNodeHelper::GetAIController(UBehaviorTreeComponent& OwnerComp)
{
	return OwnerComp.GetAIOwner();
}

UBlackboardComponent* UBTNodeHelper::GetBlackboard(UBehaviorTreeComponent& OwnerComp)
{
	return OwnerComp.GetBlackboardComponent();
}

APawn* UBTNodeHelper::GetAIPawn(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AI = GetAIController(OwnerComp);
	return AI ? AI->GetPawn() : nullptr;
}

AActor* UBTNodeHelper::GetActorFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key)
{
	UBlackboardComponent* BB = GetBlackboard(OwnerComp);
	return BB ? Cast<AActor>(BB->GetValueAsObject(Key.SelectedKeyName)) : nullptr;
}

UAbilitySystemComponent* UBTNodeHelper::GetAbilitySystemComponent(UBehaviorTreeComponent& OwnerComp)
{
	APawn* Pawn = GetAIPawn(OwnerComp);
	return Pawn ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn) : nullptr;
}
