#include "AI/BlackoutBossAIController.h"
#include "AI/BossPhaseManager.h"
#include "AI/BossBTRunner.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StateTreeAIComponent.h"

ABlackoutBossAIController::ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 컴포넌트는 Actor에 소속돼야 하므로 여기서 생성
	SubBTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("SubBehaviorTreeComp"));
	BBComp    = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ABlackoutBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // HasAuthority() 검증 후 StateTree 시작까지 처리

	if (!HasAuthority()) return;

	// 서버에서만 매니저 생성 및 초기화
	PhaseManager = NewObject<UBossPhaseManager>(this);
	PhaseManager->Initialize(this, StateTreeComp);

	BTRunner = NewObject<UBossBTRunner>(this);
	BTRunner->Initialize(this, SubBTComp, BBComp);
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

void ABlackoutBossAIController::RunSubBehaviorTree(UBehaviorTree* SubTree)
{
	if (BTRunner) BTRunner->RunBehaviorTree(SubTree);
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