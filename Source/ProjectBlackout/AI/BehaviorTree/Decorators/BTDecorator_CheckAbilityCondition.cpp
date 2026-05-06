#include "AI/BehaviorTree/Decorators/BTDecorator_CheckAbilityCondition.h"
#include "AI/BehaviorTree/BTNodeHelper.h"
#include "AI/BOAICalcHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_CheckAbilityCondition::UBTDecorator_CheckAbilityCondition()
{
	NodeName = "Check Ability Condition";
	// 조건 재평가 없음 — 진입 시점 한 번만 체크
	FlowAbortMode = EBTFlowAbortMode::None;
}

bool UBTDecorator_CheckAbilityCondition::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB) return false;

	// 1. 타겟 유효성
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target || Target->IsPendingKillPending()) return false;

	// 2. 거리 검사 (MaxRange > 0 일 때만)
	if (MaxRange > 0.f)
	{
		APawn* Pawn = UBTNodeHelper::GetAIPawn(OwnerComp);
		if (!Pawn) return false;

		if (!UBOAICalcHelper::IsWithinRange(Pawn, Target, MaxRange)) return false;
	}

	return true;
}

FString UBTDecorator_CheckAbilityCondition::GetStaticDescription() const
{
	if (MaxRange > 0.f)
	{
		return FString::Printf(TEXT("Target Valid  |  Range ≤ %.0fcm"), MaxRange);
	}
	return TEXT("Target Valid");
}
