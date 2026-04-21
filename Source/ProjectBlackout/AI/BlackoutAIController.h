#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BlackoutAIController.generated.h"

class UStateTreeAIComponent;

/**
 * Base AI Controller for Project Blackout.
 * Responsible for initializing and running the StateTree for AI characters.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABlackoutAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Initialize external data/context handles for StateTree */
	virtual void InitStateTreeContext();

	/** Initialize AI Perception if necessary */
	virtual void InitPerception();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComp;
};
