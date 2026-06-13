// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutRavagerAIController.h"

#include "BlackoutBossBTRunner.h"
#include "BlackoutPhaseEvaluator.h"
#include "BehaviorTree/BlackboardComponent.h"

void ABlackoutRavagerAIController::RequestPhaseChange(EBOBossPhase NewPhase)
{
	if (PhaseEvaluator)
	{
		PhaseEvaluator->RequestPhaseChange(NewPhase);
	}
}

EBOBossPhase ABlackoutRavagerAIController::GetCurrentPhase() const
{
	return PhaseEvaluator ? PhaseEvaluator->GetCurrentPhase() : EBOBossPhase::None;
}

void ABlackoutRavagerAIController::StartCombat()
{
	if (!HasAuthority()) return;
	
	if (!PhaseEvaluator)
	{
		UE_LOG(LogTemp, Warning, TEXT("Blackout: PhaseEvaluator is not set In ABlackoutRavagerAIController"));
		return;
	}
	PhaseEvaluator->RequestPhaseChange(EBOBossPhase::Phase1);
	
	Super::StartCombat();
}

void ABlackoutRavagerAIController::OnUnPossess()
{
	if (PhaseEvaluator)   PhaseEvaluator->Deinitialize();
	if (BTRunner)         BTRunner->StopBT();
    
	BTRunner = nullptr;
	PhaseEvaluator = nullptr;

	Super::OnUnPossess();
}

void ABlackoutRavagerAIController::PreInitialize(APawn* InPawn)
{
	Super::PreInitialize(InPawn);
	
	if (!CachedASC) return;
	
	// BTRunner
	BTRunner = NewObject<UBlackoutBossBTRunner>(this);
	BTRunner->Initialize(this, PhaseBehaviorTrees);
	
	// Phase
	PhaseEvaluator = NewObject<UBlackoutPhaseEvaluator>(this);
	PhaseEvaluator->OnBossPhaseChanged.AddUObject(this, &ABlackoutRavagerAIController::HandlePhaseChanged);
	PhaseEvaluator->Initialize(this,CachedASC);
}


void ABlackoutRavagerAIController::HandleAggroTargetChanged(APawn* NewTarget)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;
	
	BB->SetValueAsObject(TEXT("Target"), NewTarget);
}

void ABlackoutRavagerAIController::HandlePhaseChanged(EBOBossPhase NewPhase)
{
	UE_LOG(LogTemp, Warning, TEXT("HandlePhaseChanged"));
	if (BTRunner)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandlePhaseChanged BTRunner"));
		BTRunner->RunPhaseBT(NewPhase);
	}
}
