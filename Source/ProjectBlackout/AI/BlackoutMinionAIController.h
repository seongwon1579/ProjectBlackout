// ─── 구현 내역 ───────────────────────
//  - 김민영: AI 퍼셉션 감지 기반 미니언 컨트롤러 — 전투 타겟 설정, 서브 BT 없이 StateTree 단독 구동
//  - 조성원: 스폰/레벨시퀀스 요청에 맞춰 미니언 AI가 시작되도록 전투 시작 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BlackoutMinionAIController.generated.h"

/**
 * AI Controller for Minions. Runs pure StateTree without Sub-BehaviorTrees.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutMinionAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	ABlackoutMinionAIController();

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetCombatTarget(APawn* TargetPawn);
	
	virtual void StartCombat() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
};
