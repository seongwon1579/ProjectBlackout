#include "AI/BlackoutBossAIController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BlackoutGameplayTags.h"
#include "BossBTRunner.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameFramework/PlayerController.h"


ABlackoutBossAIController::ABlackoutBossAIController()
{
}

void ABlackoutBossAIController::RequestPhaseChange(EBossPhase NewPhase)
{
	if (NewPhase == EBossPhase::None) return;
	
	if (NewPhase <= CurrentPhase) return;
    
	if (PendingPhase != EBossPhase::None && NewPhase <= PendingPhase) return;
	
	
	PendingPhase = NewPhase;
	
	TryApplyPendingPhase();
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	CachedASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}

	PhaseLockTag = BlackoutGameplayTags::Ability_PhaseLock;

	BTRunner = NewObject<UBossBTRunner>(this);
	BTRunner->Initialize(this, PhaseBehaviorTrees);
	
	if (!CachedASC) return;

	PhaseLockTagChangedHandle = CachedASC->RegisterGameplayTagEvent(
		PhaseLockTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABlackoutBossAIController::OnPhaseLockTagChanged);

	RequestPhaseChange(EBossPhase::Phase1);

	CurrentTargetIndex = 0;
	GetWorldTimerManager().SetTimerForNextTick(this, &ABlackoutBossAIController::CycleTarget);

	GetWorldTimerManager().SetTimer(
		TargetCycleTimerHandle,
		this, &ABlackoutBossAIController::CycleTarget,
		10.f, true
	);
}

void ABlackoutBossAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(TargetCycleTimerHandle);

	if (CachedASC)
	{
		CachedASC->RegisterGameplayTagEvent(
			PhaseLockTag,
			EGameplayTagEventType::NewOrRemoved
		).Remove(PhaseLockTagChangedHandle);
	}

	if (BTRunner)
	{
		BTRunner->StopBT();
		BTRunner = nullptr;
	}

	Super::OnUnPossess();
}

void ABlackoutBossAIController::OnPhaseLockTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0)
	{
		TryApplyPendingPhase();
	}
}

void ABlackoutBossAIController::TryApplyPendingPhase()
{
	if (PendingPhase == EBossPhase::None) return;
	
	if (IsPhaseTransitionLocked()) return;
	
	const EBossPhase PhaseToApply = PendingPhase;
	PendingPhase = EBossPhase::None;
	
	ApplyPhaseChange(PhaseToApply);
}

void ABlackoutBossAIController::ApplyPhaseChange(EBossPhase NewPhase)
{
	CurrentPhase = NewPhase;
	
	if (BTRunner)
	{
		BTRunner->RunPhaseBT(NewPhase);
	}
}

bool ABlackoutBossAIController::IsPhaseTransitionLocked() const
{
	return CachedASC ? CachedASC->HasMatchingGameplayTag(PhaseLockTag) : false;
}


void ABlackoutBossAIController::CycleTarget()
{
	UBlackboardComponent* BlackBoard = GetBlackboardComponent();
	if (!BlackBoard) return;

	TArray<APawn*> PlayerPawns;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APawn* P = PC->GetPawn())
				PlayerPawns.Add(P);
		}
	}

	if (PlayerPawns.IsEmpty()) return;

	CurrentTargetIndex = CurrentTargetIndex % PlayerPawns.Num();
	BlackBoard->SetValueAsObject(TEXT("Target"), PlayerPawns[CurrentTargetIndex]);
	CurrentTargetIndex = (CurrentTargetIndex + 1) % PlayerPawns.Num();
}
