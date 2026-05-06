#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BTNodeHelper.generated.h"

class UAbilitySystemComponent;

/**
 * BT 노드(Decorator / Service / Task)에서 반복되는
 * AIController · Blackboard · Pawn · Actor 조회 로직을 모아둔 정적 헬퍼.
 *
 * 기본형은 베이스 타입을 반환하고, 템플릿 오버로드로 원하는 타입으로 캐스팅한다.
 *   AAIController*           → GetAIController(OwnerComp)
 *   AMyAIController*         → GetAIController<AMyAIController>(OwnerComp)
 */
UCLASS()
class PROJECTBLACKOUT_API UBTNodeHelper : public UObject
{
	GENERATED_BODY()

public:
	// ── AI Controller ────────────────────────────────────────────────────────
	static AAIController* GetAIController(UBehaviorTreeComponent& OwnerComp);

	template <typename T>
	static T* GetAIController(UBehaviorTreeComponent& OwnerComp)
	{
		return Cast<T>(GetAIController(OwnerComp));
	}

	// ── Blackboard ───────────────────────────────────────────────────────────
	static UBlackboardComponent* GetBlackboard(UBehaviorTreeComponent& OwnerComp);

	// ── AI Pawn ──────────────────────────────────────────────────────────────
	static APawn* GetAIPawn(UBehaviorTreeComponent& OwnerComp);

	template <typename T>
	static T* GetAIPawn(UBehaviorTreeComponent& OwnerComp)
	{
		return Cast<T>(GetAIPawn(OwnerComp));
	}

	// ── Actor from Blackboard ────────────────────────────────────────────────
	static AActor* GetActorFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key);

	template <typename T>
	static T* GetActorFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key)
	{
		return Cast<T>(GetActorFromBB(OwnerComp, Key));
	}
	
	// ── Float from Blackboard ────────────────────────────────────────────────
	static TOptional<float> GetFloatFromBB(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key);
	
	// ── AbilitySystemComponent ───────────────────────────────────────────────
	static UAbilitySystemComponent* GetAbilitySystemComponent(UBehaviorTreeComponent& OwnerComp);
	
	template <typename T>
	static T* GetAbilitySystemComponent(UBehaviorTreeComponent& OwnerComp)
	{
		return Cast<T>(GetAbilitySystemComponent(OwnerComp));
	}
};