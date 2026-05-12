#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTT_ActivateAbility.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

	
public:
	UBTT_ActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString             GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability",
		meta = (EditCondition = "!bReadTagFromBlackboard"))
	FGameplayTag AbilityTag;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bReadTagFromBlackboard = false;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability",
		meta = (EditCondition = "bReadTagFromBlackboard"))
	FBlackboardKeySelector SelectedGameAbilityTagKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bWaitForEnd = true;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bPassSignedAngle = false;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector CurrentTargetKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SignedAngleKey;

private:
	/** 태그 소스(고정 or BB)에 따라 최종 AbilityTag를 반환한다. */
	FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const;
	
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityEnded(UGameplayAbility* Ability);
	void UnbindDelegate();

	UBehaviorTreeComponent*                 CachedOwnerComp = nullptr;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<UGameplayAbility>        BoundAbility;
};