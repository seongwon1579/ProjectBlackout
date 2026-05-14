// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_CanEvade.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBTD_CanEvade : public UBTDecorator
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Condition", meta = (ClampMin = "0.0"))
	float TraceLength = 500.f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
	bool IsBlocked(const APawn* Owner, const FVector& Direction) const ;
	
	UPROPERTY(EditAnywhere, Category= "Blackout|Blackboard|Input")
	FBlackboardKeySelector EvadeDirectionKey;

};
