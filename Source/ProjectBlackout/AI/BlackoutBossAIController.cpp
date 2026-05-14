#include "AI/BlackoutBossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/PlayerController.h"

// #include "AI/ActionPipeline.h"
// #include "AI/BossPhaseManager.h"
// #include "AI/BossBTRunner.h"
// #include "BehaviorTree/BehaviorTreeComponent.h"
// #include "BehaviorTree/BlackboardComponent.h"
// #include "Components/StateTreeAIComponent.h"

ABlackoutBossAIController::ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SetPerceptionComponent(*PerceptionComp);

	// CurrentBTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("SubBehaviorTreeComp"));
	// BBComp        = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	// StateTree는 건너뜀 — Super 호출 생략
	// Super::OnPossess(InPawn);

	// AAIController::OnPossess 직접 호출 (StateTree 없이)
	AAIController::OnPossess(InPawn);

	InitPerception();

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);

		CurrentTargetIndex = 0;
		GetWorldTimerManager().SetTimerForNextTick(this, &ABlackoutBossAIController::CycleTarget);

		GetWorldTimerManager().SetTimer(
			TargetCycleTimerHandle,
			this, &ABlackoutBossAIController::CycleTarget,
			10.f, true
		);
	}

	// PhaseManager = NewObject<UBossPhaseManager>(this);
	// PhaseManager->Initialize(this, StateTreeComp);
	// BTRunner = NewObject<UBossBTRunner>(this);
	// BTRunner->Initialize(this, CurrentBTComp, BBComp);
	// BrainComponent = CurrentBTComp;
}

void ABlackoutBossAIController::OnUnPossess()
{
	// if (PhaseManager) PhaseManager->Stop(TEXT("UnPossessed"));
	// if (BTRunner)     BTRunner->Stop();

	GetWorldTimerManager().ClearTimer(TargetCycleTimerHandle);
	AAIController::OnUnPossess();
}

// ── Target Cycling ────────────────────────────────────────────────────────────

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

// ── Perception ────────────────────────────────────────────────────────────────

void ABlackoutBossAIController::InitPerception()
{
	UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(this);
	SightConfig->SightRadius                          = 2000.f;
	SightConfig->LoseSightRadius                      = 2500.f;
	SightConfig->PeripheralVisionAngleDegrees         = 90.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABlackoutBossAIController::OnTargetPerceived);
}

void ABlackoutBossAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
	// if (!Stimulus.WasSuccessfullySensed()) return;
	// if (APawn* TargetPawn = Cast<APawn>(Actor))
	// {
	// 	WriteTargetToBlackboard(TargetPawn);
	// }
}

// ── 비활성화된 BT 퍼사드 ─────────────────────────────────────────────────────

// void ABlackoutBossAIController::RunSubBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget)
// {
// 	if (BTRunner) BTRunner->RunBehaviorTree(SubTree, InitialTarget);
// }
//
// void ABlackoutBossAIController::StopSubBehaviorTree()
// {
// 	if (BTRunner) BTRunner->Stop();
// }
//
// bool ABlackoutBossAIController::IsSubBehaviorTreeRunning() const
// {
// 	return BTRunner && BTRunner->IsRunning();
// }
//
// void ABlackoutBossAIController::WriteTargetToBlackboard(APawn* TargetPawn)
// {
// 	if (BTRunner) BTRunner->WriteTargetToBlackboard(TargetPawn);
// }
//
// void ABlackoutBossAIController::RequestPhaseExit()
// {
// 	if (BTRunner) BTRunner->RequestPhaseExit();
// }
