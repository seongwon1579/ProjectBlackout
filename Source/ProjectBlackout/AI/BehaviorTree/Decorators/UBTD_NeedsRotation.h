// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UBTD_NeedsRotation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UUBTD_NeedsRotation : public UBTDecorator
{
	GENERATED_BODY()

public:
	UUBTD_NeedsRotation();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SignedAngleKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Params")
	float AngleThreshold;
};
