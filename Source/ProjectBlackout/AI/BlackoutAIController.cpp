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
	
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StartLogic();
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
