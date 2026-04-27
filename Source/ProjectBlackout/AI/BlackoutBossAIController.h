#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "AI/ActionPipelineOwner.h"
#include "AI/BossPhaseManager.h"
#include "AI/BossBTRunner.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BlackoutBossAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class UActionPipeline;

/**
 * 보스 전용 AI 컨트롤러.
 *
 * 책임은 두 UObject 매니저에 위임된다:
 *   UBossPhaseManager — StateTree 페이즈 생명주기
 *   UBossBTRunner     — 페이즈별 BehaviorTree 실행 및 Blackboard 관리
 *
 * 이 클래스는 외부(StateTree Task 등)에 대한 퍼사드(Facade) 역할만 한다.
 * 서버(Dedicated Server) 전용: 모든 AI 로직은 서버에서만 실행된다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController, public IActionPipelineOwner
{
	GENERATED_BODY()

public:
	ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess  (APawn* InPawn) override;
	virtual void OnUnPossess()              override;

	// ── BT 퍼사드 (FBSTTask_RunSubBehaviorTree 에서 호출) ─────────────────────
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void RunSubBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void StopSubBehaviorTree();

	/** FBSTTask_RunSubBehaviorTree::Tick 에서 BT 자체 종료를 감지하는 데 사용 */
	bool IsSubBehaviorTreeRunning() const;

	// ── Blackboard 퍼사드 ─────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void WriteTargetToBlackboard(APawn* TargetPawn);

	/** BT를 즉시 중단하지 않고 현재 어빌리티 완료 후 안전하게 종료 유도 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void RequestPhaseExit();

	// ── IActionPipelineOwner ──────────────────────────────────────────────────
	virtual UActionPipeline* GetActionPipeline() const override { return ActionPipeline; }

protected:
	virtual void InitPerception() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

	// Actor에 소속된 컴포넌트 (GC 및 네트워크 컨텍스트 필요)
	UPROPERTY(VisibleAnywhere, Category = "Blackout|AI")
	TObjectPtr<UBehaviorTreeComponent> SubBTComp;

	UPROPERTY(VisibleAnywhere, Category = "Blackout|AI")
	TObjectPtr<UBlackboardComponent> BBComp;

	UPROPERTY(VisibleAnywhere, Category = "Blackout|AI")
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	// 책임 분리된 로직 매니저 (서버 전용 UObject, Outer = this)
	UPROPERTY()
	TObjectPtr<UBossPhaseManager> PhaseManager;

	UPROPERTY()
	TObjectPtr<UBossBTRunner> BTRunner;

	UPROPERTY()
	TObjectPtr<UActionPipeline> ActionPipeline;
};