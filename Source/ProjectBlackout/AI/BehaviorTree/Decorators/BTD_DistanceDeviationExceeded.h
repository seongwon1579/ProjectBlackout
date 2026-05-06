// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTD_DistanceDeviationExceeded.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UBTD_DistanceDeviationExceeded : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTD_DistanceDeviationExceeded();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;

	UPROPERTY(EditDefaultsOnly, Category="Blackout|Config")
	uint8 ChaseStateIndex = 0;

private:

	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	bool bLastResult = false;
	bool bHasLastResult = false;
};
