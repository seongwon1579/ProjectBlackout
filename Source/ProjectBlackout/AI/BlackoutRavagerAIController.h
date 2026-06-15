// Fill out your copyright notice in the Description page of Project Settings.

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
