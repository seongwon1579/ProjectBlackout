// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_LineTraceCheck.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_LineTraceCheck : public UBTDecorator
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Condition", meta =(ClampMin = "0.0"))
	float TraceLength = 500.f;
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
};
