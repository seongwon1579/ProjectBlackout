#include "AI/BlackoutBossAIController.h"
#include "AI/ActionPipeline.h"
#include "AI/BossPhaseManager.h"
#include "AI/BossBTRunner.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABlackoutBossAIController::ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SubBTComp      = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("SubBehaviorTreeComp"));
	BBComp         = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SetPerceptionComponent(*PerceptionComp);
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // HasAuthority() 검증 후 StateTree 시작까지 처리

	//if (!HasAuthority()) return;

	// 서버에서만 매니저 생성 및 초기화
	PhaseManager = NewObject<UBossPhaseManager>(this);
	PhaseManager->Initialize(this, StateTreeComp);

	BTRunner = NewObject<UBossBTRunner>(this);
	BTRunner->Initialize(this,SubBTComp, BBComp);

	ActionPipeline = NewObject<UActionPipeline>(this);
	ActionPipeline->Initialize();
}

void ABlackoutBossAIController::OnUnPossess()
{
	if (PhaseManager)
	{
		PhaseManager->Stop(TEXT("UnPossessed"));
	}
	if (BTRunner)
	{
		BTRunner->Stop();
	}

	Super::OnUnPossess();
}

// ── BT 퍼사드 ─────────────────────────────────────────────────────────────────

void ABlackoutBossAIController::RunSubBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget)
{
	if (BTRunner)
	{
		BTRunner->RunBehaviorTree(SubTree, InitialTarget);
	}
}

void ABlackoutBossAIController::StopSubBehaviorTree()
{
	if (BTRunner) BTRunner->Stop();
}

bool ABlackoutBossAIController::IsSubBehaviorTreeRunning() const
{
	return BTRunner && BTRunner->IsRunning();
}

// ── Blackboard 퍼사드 ─────────────────────────────────────────────────────────

void ABlackoutBossAIController::WriteTargetToBlackboard(APawn* TargetPawn)
{
	if (BTRunner) BTRunner->WriteTargetToBlackboard(TargetPawn);
}

void ABlackoutBossAIController::RequestPhaseExit()
{
	if (BTRunner) BTRunner->RequestPhaseExit();
}

// ── Perception ────────────────────────────────────────────────────────────────

void ABlackoutBossAIController::InitPerception()
{
	UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(this);
	SightConfig->SightRadius                          = 2000.f;
	SightConfig->LoseSightRadius                      = 2500.f;
	SightConfig->PeripheralVisionAngleDegrees         = 90.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABlackoutBossAIController::OnTargetPerceived);
}

void ABlackoutBossAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (BTRunner->CheckingActor)
	{
	}
}

void ABlackoutBossAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;
	if (APawn* TargetPawn = Cast<APawn>(Actor))
	{
		WriteTargetToBlackboard(TargetPawn);
	}
}
