#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BlackoutMinionAIController.generated.h"

/**
 * AI Controller for Minions. Runs pure StateTree without Sub-BehaviorTrees.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutMinionAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	ABlackoutMinionAIController();

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetCombatTarget(APawn* TargetPawn);
	
	virtual void StartCombat() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
};
