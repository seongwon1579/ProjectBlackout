// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 블랙보드 거리 값을 Min/Max 범위와 비교해 조건을 판정하는 거리 체크 데코레이터
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsInRange.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_IsInRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_IsInRange();
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
	bool IsInRange(float Distance, float Min, float Max) const;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector DistanceBTWActors;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Params")
	float MinRange = -1.f;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Params")
	float MaxRange = -1.f;
};
