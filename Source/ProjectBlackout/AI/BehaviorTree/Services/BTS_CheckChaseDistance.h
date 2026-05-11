// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_CheckChaseDistance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTS_CheckChaseDistance : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_CheckChaseDistance();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Input")
	FBlackboardKeySelector DistanceBTWActors;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Output")
	FBlackboardKeySelector OnTrigger;

protected:
	bool IsActivation(UBehaviorTreeComponent& OwnerComp);

	UPROPERTY(EditAnywhere, Category = "Blackout|Parmas")
	float Threshold;
	
	bool bOnService;
};
