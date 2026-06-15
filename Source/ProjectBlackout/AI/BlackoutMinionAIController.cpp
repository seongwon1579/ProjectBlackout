#include "BlackoutMinionAIController.h"

#include "Components/StateTreeAIComponent.h"
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

void ABlackoutMinionAIController::StartCombat()
{
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StartLogic();
	}
}

void ABlackoutMinionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	StartCombat();
}

void ABlackoutMinionAIController::OnUnPossess()
{
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StopLogic("UnPossessed");
	}
	Super::OnUnPossess();
}
