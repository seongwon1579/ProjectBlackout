// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsInRange.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_IsInRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_IsInRange();
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
	bool IsInRange(float Distance, float Min, float Max) const;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector DistanceBTWActors;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Params")
	float MinRange = -1.f;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Params")
	float MaxRange = -1.f;
};
