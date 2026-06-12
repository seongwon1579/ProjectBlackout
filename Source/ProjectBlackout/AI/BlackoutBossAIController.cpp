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
	
	UE_LOG(LogTemp, Warning, TEXT("ABlackoutBossAIController OnPossess"))
	
	PreInitialize(InPawn);
	
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

void ABlackoutBossAIController::PreInitialize(APawn* InPawn)
{
	CachedASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}
}
