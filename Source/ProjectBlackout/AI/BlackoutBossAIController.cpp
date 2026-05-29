#include "AI/BlackoutBossAIController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BlackoutAggroEvaluator.h"
#include "BlackoutPhaseEvaluator.h"
#include "BlackoutBossBTRunner.h"
#include "GameFramework/PlayerController.h"

void ABlackoutBossAIController::RequestPhaseChange(EBOBossPhase NewPhase)
{
	if (PhaseEvaluator)
	{
		PhaseEvaluator->RequestPhaseChange(NewPhase);
	}
}

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
	if (!CachedASC) return;
	// --------------------------------------------------------------------------
	BTRunner = NewObject<UBlackoutBossBTRunner>(this);
	BTRunner->Initialize(this, PhaseBehaviorTrees);
	
	// Aggro
	AggroEvaluator = NewObject<UBlackoutAggroEvaluator>(this);
	AggroEvaluator->Initialize(this, CachedASC);
	
	// Phase
	PhaseEvaluator = NewObject<UBlackoutPhaseEvaluator>(this);
	PhaseEvaluator->OnBossPhaseChanged.AddUObject(this, &ABlackoutBossAIController::HandlePhaseChanged);
	PhaseEvaluator->Initialize(this, CachedASC);
}

void ABlackoutBossAIController::OnUnPossess()
{
	if (PhaseEvaluator)   PhaseEvaluator->Deinitialize();
	if (AggroEvaluator)   AggroEvaluator->Deinitialize();
	if (BTRunner)         BTRunner->StopBT();
    
	CachedASC = nullptr;
	BTRunner = nullptr;
	AggroEvaluator = nullptr;
	PhaseEvaluator = nullptr;

	Super::OnUnPossess();
}

void ABlackoutBossAIController::HandlePhaseChanged(EBOBossPhase NewPhase)
{
	if (BTRunner)
	{
		BTRunner->RunPhaseBT(NewPhase);
	}
}

EBOBossPhase ABlackoutBossAIController::GetCurrentPhase() const
{
	return PhaseEvaluator ? PhaseEvaluator->GetCurrentPhase() : EBOBossPhase::None;
}
