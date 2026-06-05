// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SelectAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_SelectAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_SelectAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	FGameplayTag AbilityTagToSet;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector SelectedAbilityTagKey;
};
