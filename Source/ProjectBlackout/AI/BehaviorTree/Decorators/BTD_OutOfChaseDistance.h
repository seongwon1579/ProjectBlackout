// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_OutOfChaseDistance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_OutOfChaseDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTD_OutOfChaseDistance();
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
	protected:
	
	EBlackboardNotificationResult OnBlackboardKeyChanged(const UBlackboardComponent& BB, FBlackboard::FKey KeyID);
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector OnTrigger;
	
};
