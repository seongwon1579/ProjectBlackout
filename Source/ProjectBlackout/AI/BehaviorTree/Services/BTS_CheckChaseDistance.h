// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 패턴 데이터에서 받은 사거리로 공격 가능 범위 도달 여부를 갱신하는 추격 거리 서비스 (보스 일반화 포함)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BORavagerPatternData.h"
#include "BehaviorTree/BTService.h"
#include "BTS_CheckChaseDistance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTS_CheckChaseDistance : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_CheckChaseDistance();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Input")
	FBlackboardKeySelector DistanceBTWActors;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Input")
	FBlackboardKeySelector Target;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Input")
	FBlackboardKeySelector ActiveAbilityTagKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blacboard|Output")
	FBlackboardKeySelector bOnTrigger;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard|Blackboard|Ouput")
	FBlackboardKeySelector AcceptanceRadiusKey;

protected:
	
	UPROPERTY(Transient)
	FBossChaseRanges CachedRanges;
};
