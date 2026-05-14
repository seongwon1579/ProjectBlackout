// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"

#include "BTT_ActivateRotateAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateRotateAbility : public UBTT_ActivateAbility
{
	GENERATED_BODY()
	
public:
	UBTT_ActivateRotateAbility();
	
protected:
	virtual void PrepareEventData(FGameplayEventData& EventData, UBlackboardComponent* BB) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SignedAngleKey;

};
