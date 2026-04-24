#include "AI/BossPhaseManager.h"
#include "AI/BlackoutBossAIController.h"
#include "Components/StateTreeAIComponent.h"

void UBossPhaseManager::Initialize(ABlackoutBossAIController* InOwner, UStateTreeAIComponent* InStateTreeComp)
{
	OwnerController = InOwner;
	StateTreeComp   = InStateTreeComp;
}

void UBossPhaseManager::Start()
{
	if (!HasAuthority() || !StateTreeComp.IsValid()) return;
	StateTreeComp->StartLogic();
}

void UBossPhaseManager::Stop(const FString& Reason)
{
	if (!StateTreeComp.IsValid()) return;
	StateTreeComp->StopLogic(Reason);
}

bool UBossPhaseManager::HasAuthority() const
{
	return OwnerController.IsValid() && OwnerController->HasAuthority();
}