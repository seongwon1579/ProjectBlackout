#include "AI/BlackoutBossAIController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BlackoutAggroEvaluator.h"
#include "GameFramework/PlayerController.h"

void ABlackoutBossAIController::RecordDamage(APawn* Source, float Amount)
{
	if (AggroEvaluator)
	{
		AggroEvaluator->RecordDamage(Source, Amount);
	}
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}
	
	// Aggro
	AggroEvaluator = NewObject<UBlackoutAggroEvaluator>(this);
	AggroEvaluator->OnAggroTargetChanged.AddUObject(this,&ABlackoutBossAIController::HandleAggroTargetChanged);
	AggroEvaluator->Initialize(this, CachedASC);
	
}

void ABlackoutBossAIController::OnUnPossess()
{
	if (AggroEvaluator)   AggroEvaluator->Deinitialize();
	
	CachedASC = nullptr;
	AggroEvaluator = nullptr;
	
	Super::OnUnPossess();
}
