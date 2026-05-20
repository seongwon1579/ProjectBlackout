#include "BlackoutMinionAIController.h"
#include "Perception/AIPerceptionComponent.h"

ABlackoutMinionAIController::ABlackoutMinionAIController()
{
}


void ABlackoutMinionAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Handle perception updates and potentially set combat target in StateTree context/blackboard
}

void ABlackoutMinionAIController::SetCombatTarget(APawn* TargetPawn)
{
	// Set the target for the minion
}
