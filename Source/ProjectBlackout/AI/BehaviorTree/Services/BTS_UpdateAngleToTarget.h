// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 타겟 방향까지의 부호각을 매 틱 계산해 블랙보드에 기록하는 서비스
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_UpdateAngleToTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTS_UpdateAngleToTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTS_UpdateAngleToTarget();
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector SignedAngleKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector TargetKey;
};
