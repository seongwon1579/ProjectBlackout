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

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	bool bWaitForEnd = true;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector TargetKey;

	virtual FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const;

	virtual void PrepareEventData(FGameplayEventData& EventData, UBlackboardComponent* BB){}

private:
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityEnded(UGameplayAbility* Ability);
	void UnbindDelegate();

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	FGameplayTag AbilityTag;

	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<UGameplayAbility> BoundAbility;
};
