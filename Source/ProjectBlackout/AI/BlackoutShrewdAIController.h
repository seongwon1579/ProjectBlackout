// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Shrewd 보스 컨트롤러 — 현재 어그로 타겟 캐싱 및 StateTree로의 타겟 전달
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutBossAIController.h"
#include "BlackoutShrewdAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutShrewdAIController : public ABlackoutBossAIController
{
	GENERATED_BODY()

public:
	APawn* GetCurrentAggroTarget() const { return CurrentAggroTarget; }
	
	virtual void StartCombat() override;
	virtual void OnUnPossess() override;
	
protected:
	APawn* CurrentAggroTarget;
	virtual void HandleAggroTargetChanged(APawn* NewTarget) override;
};
