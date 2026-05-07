// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_RandomChance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_RandomChance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTD_RandomChance();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Parmas", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Random = 0.5f;
};
