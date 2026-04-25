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
	ABlackoutMinionAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetCombatTarget(APawn* TargetPawn);

protected:
	virtual void InitStateTreeContext() override;
	virtual void InitPerception() override;
};
