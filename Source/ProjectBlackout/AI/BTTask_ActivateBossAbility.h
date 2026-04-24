#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateBossAbility.generated.h"

class UGameplayAbility;

/**
 * 보스 GA를 실행하고 종료를 대기하는 BT Task.
 *
 * - bCreateNodeInstance = true: 실행마다 독립 인스턴스 생성 → 멤버 변수로 상태 보관.
 * - Tick 없음: GA 종료는 OnGameplayAbilityEnded 델리게이트로 감지.
 * - AbortTask: 어빌리티를 취소하지 않음 — 현재 동작을 자연스럽게 마무리하도록 허용.
 *   (AbortTask 후에도 GA는 끝까지 실행되고, 다음 BT 노드만 실행되지 않는다.)
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
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bWaitForEnd = true;

private:
	void HandleAbilityEnded(UGameplayAbility* Ability);
	void UnbindDelegate();

	UBehaviorTreeComponent*        CachedOwnerComp = nullptr;
	TWeakObjectPtr<UGameplayAbility> BoundAbility;
};