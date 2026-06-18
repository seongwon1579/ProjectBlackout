// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 블랙보드 부호각이 임계치를 넘으면 회전이 필요하다고 판정하는 데코레이터
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_NeedsRotation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTD_NeedsRotation : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTD_NeedsRotation();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SignedAngleKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Params")
	float AngleThreshold;
};
