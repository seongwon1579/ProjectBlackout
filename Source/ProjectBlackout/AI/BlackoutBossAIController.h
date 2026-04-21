#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "BlackoutBossAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

/**
 * AI Controller for Bosses. Uses StateTree for Phase management and 
 * BehaviorTree for sub-phase attack patterns.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess(APawn* InPawn) override;

	/** Runs a sub-behavior tree for the current phase */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void RunSubBehaviorTree(UBehaviorTree* SubTree);

	/** Stops the currently running sub-behavior tree */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void StopSubBehaviorTree();

	/** Writes the given target to the blackboard (called by Aggro Evaluator) */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void WriteTargetToBlackboard(APawn* TargetPawn);

protected:
	virtual void InitStateTreeContext() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UBehaviorTreeComponent> SubBehaviorTreeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UBlackboardComponent> BlackboardComp;
};
