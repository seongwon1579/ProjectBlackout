#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTargetData.generated.h"

/**
 * 타겟 위치, 거리, 방향 각도를 블랙보드에 갱신하는 서비스.
 * AI → Target 벡터 기준으로 Distance(float), TargetLocation(vector), DirectionAngle(float) 을 씁니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTService_UpdateTargetData : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateTargetData();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// /** 추적할 타겟 액터 */
	// UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	// FBlackboardKeySelector TargetKey;

	/** 타겟 월드 위치 (Vector) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector OutLocationKey;

	/** AI ~ 타겟 2D 거리 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector OutDistanceKey;

	/** AI 전방 기준 타겟 방향 각도 -180~180 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector OutDirectionAngleKey;
};