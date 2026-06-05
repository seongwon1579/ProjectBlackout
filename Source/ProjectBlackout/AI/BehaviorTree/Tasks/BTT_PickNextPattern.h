// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PickNextPattern.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_PickNextPattern : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_PickNextPattern();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|PatternNumber")
	int32 PatternNumber;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector PatternNumberKey;
};
