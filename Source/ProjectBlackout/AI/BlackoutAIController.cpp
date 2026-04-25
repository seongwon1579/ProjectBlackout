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
			// StateTree 에셋은 BP 서브클래스 또는 DataAsset에서 지정된 후 이 호출이 이루어져야 함
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
	// 베이스 구현. 서브클래스에서 오버라이드하여 외부 데이터 핸들 바인딩.
}

void ABlackoutAIController::InitPerception()
{
	// 베이스 구현. Perception이 필요한 서브클래스에서 오버라이드.
}
