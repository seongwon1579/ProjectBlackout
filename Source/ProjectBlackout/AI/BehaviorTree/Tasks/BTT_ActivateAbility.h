// ─── 구현 내역 ───────────────────────
//  - 조성원: 태그/블랙보드로 어빌리티를 실행하고 시작·종료 델리게이트를 구독해 완료까지 대기하는 베이스 Task
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "BTT_ActivateAbility.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ActivateAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const;
	virtual void PrepareEventData(FGameplayEventData& EventData, UBlackboardComponent* BB){}
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bWaitForEnd = true;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector TargetKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input", meta = (EditCondition = "!bUseAbilityTagKey", EditConditionHides))
	FBlackboardKeySelector SelectedAbilityTagKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bUseAbilityTagKey = true;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability", meta = (Categories = "Ability", EditCondition = "bUseAbilityTagKey", EditConditionHides))
	FGameplayTag AbilityTag;
	
private:
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityEnded(UGameplayAbility* Ability);
	void UnbindDelegate();
	
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<UGameplayAbility> BoundAbility;
};
