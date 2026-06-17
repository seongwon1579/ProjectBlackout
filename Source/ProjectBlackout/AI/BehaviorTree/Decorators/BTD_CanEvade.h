// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 라인 트레이스로 좌/우 회피 가능 방향을 판정하는 데코레이터 + 회피 가능 영역 디버그 시각화
//  - 김민영: 회피 판정 디버그 드로우 토글 추가
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_CanEvade.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBTD_CanEvade : public UBTDecorator
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Condition", meta = (ClampMin = "0.0"))
	float TraceLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bEnableDebugDraw = false;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
	bool IsBlocked(const APawn* Owner, const FVector& Direction) const ;
	
	UPROPERTY(EditAnywhere, Category= "Blackout|Blackboard|Input")
	FBlackboardKeySelector EvadeDirectionKey;

};
