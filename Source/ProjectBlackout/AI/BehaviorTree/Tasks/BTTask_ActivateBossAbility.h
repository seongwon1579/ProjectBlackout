#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateBossAbility.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * 보스 GA를 실행하고 종료를 대기하는 BT Task.
 *
 * - bCreateNodeInstance = true: 실행마다 독립 인스턴스 생성 → 멤버 변수로 상태 보관.
 * - Tick 없음: GA 종료는 OnGameplayAbilityEnded 델리게이트로 감지.
 * - AbortTask: 어빌리티를 취소하지 않음 — 현재 동작을 자연스럽게 마무리하도록 허용.
 *   (AbortTask 후에도 GA는 끝까지 실행되고, 다음 BT 노드만 실행되지 않는다.)
 *
 * [태그 소스]
 * - bReadTagFromBlackboard = false: AbilityTag UPROPERTY를 직접 사용.
 * - bReadTagFromBlackboard = true : AbilityTagKey 블랙보드 키(Name)에서 읽음.
 *   BTTask_SelectPattern과 연계할 때 사용한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_ActivateBossAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateBossAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString             GetStaticDescription() const override;

protected:
	/** bReadTagFromBlackboard = false일 때 사용하는 고정 어빌리티 태그. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability",
		meta = (EditCondition = "!bReadTagFromBlackboard"))
	FGameplayTag AbilityTag;

	/** true면 AbilityTagKey 블랙보드 키에서 태그를 읽는다. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bReadTagFromBlackboard = false;

	/** bReadTagFromBlackboard = true일 때 읽어올 블랙보드 키 (Name 타입). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability",
		meta = (EditCondition = "bReadTagFromBlackboard"))
	FBlackboardKeySelector SelectedGameAbilityTagKey;

	/** MotionWarp 대상 타겟 블랙보드 키 (Object). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector CurrentTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bWaitForEnd = true;

private:
	/** 태그 소스(고정 or BB)에 따라 최종 AbilityTag를 반환한다. */
	FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const;

	/**
	 * ASC::AbilityActivatedCallbacks 콜백.
	 * HandleGameplayEvent 트리거 직후 활성화된 GA 인스턴스를 확보하고
	 * OnGameplayAbilityEnded를 바인딩한다. 일회성으로 동작한다.
	 */
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityEnded(UGameplayAbility* Ability);
	void UnbindDelegate();

	UBehaviorTreeComponent*                 CachedOwnerComp = nullptr;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<UGameplayAbility>        BoundAbility;
};