// ─── 구현 내역 ───────────────────────
//  - 조성원: BT 노드에서 컨트롤러/폰/블랙보드 값/ASC를 일관되게 가져오는 정적 헬퍼 (템플릿 캐스팅 포함)
// ──────────────────────────────────────

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

UCLASS()
class PROJECTBLACKOUT_API UBTNodeHelper : public UObject
{
	GENERATED_BODY()

public:
	
	// ── AI Controller ──────────────────────────────────────────────────────────────
	template <typename T>
	static T* GetAIController(UBehaviorTreeComponent& OwnerComp)
	{
		return Cast<T>(OwnerComp.GetAIOwner());
	}
	
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