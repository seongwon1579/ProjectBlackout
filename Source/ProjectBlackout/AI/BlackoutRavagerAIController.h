// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 보스 컨트롤러 — 페이즈별 BT 러너 구동, 페이즈 전환 요청 처리, 어그로 타겟 반영
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutBossAIController.h"
#include "AI/BlackoutAIController.h"
#include "Enum/BOBossPhase.h"
#include "BlackoutRavagerAIController.generated.h"

/**
 * 
 */
class UBlackoutAggroEvaluator;
class UBlackoutPhaseEvaluator;
class UBlackoutBossBTRunner;
class UAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutRavagerAIController : public ABlackoutBossAIController
{
	GENERATED_BODY()

public:
	// 외부(체력 이벤트 등)에서 페이즈 전환 요청
	UFUNCTION()
	void RequestPhaseChange(EBOBossPhase NewPhase);
	
	EBOBossPhase GetCurrentPhase() const;
	
	virtual void StartCombat() override;

protected:
	virtual void OnUnPossess() override;
	virtual void PreInitialize(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|BT")
	TMap<EBOBossPhase, TObjectPtr<UBehaviorTree>> PhaseBehaviorTrees;

	virtual void HandleAggroTargetChanged(APawn* NewTarget) override;

private:
	void HandlePhaseChanged(EBOBossPhase NewPhase);

	UPROPERTY(Transient)
	TObjectPtr<UBlackoutBossBTRunner> BTRunner;

	UPROPERTY(Transient)
	TObjectPtr<UBlackoutPhaseEvaluator> PhaseEvaluator;
};
