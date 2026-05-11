#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "AI/ActionPipelineOwner.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BlackoutBossAIController.generated.h"

class UBehaviorTree;
class UActionPipeline;
// class UBehaviorTreeComponent;
// class UBlackboardComponent;
// class UBossPhaseManager;
// class UBossBTRunner;

/**
 * 보스 전용 AI 컨트롤러.
 * 에디터에서 BehaviorTreeAsset을 지정하면 OnPossess 시 자동 실행된다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController, public IActionPipelineOwner
{
	GENERATED_BODY()

public:
	ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess  (APawn* InPawn) override;
	virtual void OnUnPossess()              override;

	// ── IActionPipelineOwner ──────────────────────────────────────────────────
	virtual UActionPipeline* GetActionPipeline() const override { return ActionPipeline; }

	// ── BT 퍼사드 (추후 StateTree Task에서 호출 용) ───────────────────────────
	// UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	// void RunSubBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget = nullptr);

	// UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	// void StopSubBehaviorTree();

	// bool IsSubBehaviorTreeRunning() const;

	// UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	// void WriteTargetToBlackboard(APawn* TargetPawn);

	// UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	// void RequestPhaseExit();

protected:
	virtual void InitPerception() override;

	/** 에디터에서 실행할 BehaviorTree 에셋 지정 */
	UPROPERTY(EditAnywhere, Category = "Blackout|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, Category = "Blackout|AI")
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

private:
	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY()
	TObjectPtr<UActionPipeline> ActionPipeline;

	// ── 비활성화된 매니저들 ────────────────────────────────────────────────────
	// UPROPERTY() TObjectPtr<UBossPhaseManager> PhaseManager;
	// UPROPERTY() TObjectPtr<UBossBTRunner>     BTRunner;
	// UPROPERTY(VisibleAnywhere) TObjectPtr<UBehaviorTreeComponent> CurrentBTComp;
	// UPROPERTY(VisibleAnywhere) TObjectPtr<UBlackboardComponent>   BBComp;
};
