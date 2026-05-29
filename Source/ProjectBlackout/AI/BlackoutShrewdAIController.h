// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutBossAIController.h"
#include "BlackoutShrewdAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutShrewdAIController : public ABlackoutBossAIController
{
	GENERATED_BODY()

public:
	
	APawn* GetCurrentAggroTarget() const { return CurrentAggroTarget; }
	
protected:
	
	APawn* CurrentAggroTarget;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	virtual void HandleAggroTargetChanged(APawn* NewTarget) override;
	
};
