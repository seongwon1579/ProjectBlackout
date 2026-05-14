// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_CanEvadeSide.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBTD_CanEvadeSide : public UBTDecorator
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Condition", meta = (ClampMin = "0.0"))
	float TraceLength = 500.f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

};
