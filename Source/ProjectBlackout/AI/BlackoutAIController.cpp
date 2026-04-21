#include "BlackoutAIController.h"
#include "Components/StateTreeAIComponent.h"

ABlackoutAIController::ABlackoutAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StateTreeComp = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComp"));
}

void ABlackoutAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		InitPerception();
		InitStateTreeContext();
		
		if (StateTreeComp)
		{
			// StateTree asset should be set in Blueprint or via DataAsset before this
			StateTreeComp->StartLogic();
		}
	}
}

void ABlackoutAIController::OnUnPossess()
{
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StopLogic("UnPossessed");
	}
	
	Super::OnUnPossess();
}

void ABlackoutAIController::InitStateTreeContext()
{
	// Base implementation. Subclasses can override to bind external data handles.
}

void ABlackoutAIController::InitPerception()
{
	// Base implementation for Perception initialization.
}
