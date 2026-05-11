#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_UpdateTargetData.generated.h"

/**
 * AggroComponent의 최고 위협 타겟을 BB_CurrentTarget에 갱신하고
 * 타겟 위치, 거리, 방향 각도를 블랙보드에 쓰는 서비스.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTS_UpdateTargetData : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_UpdateTargetData();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 어그로 타겟을 쓸 블랙보드 키 (Object) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 타겟 월드 위치 (Vector) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	/** AI ~ 타겟 2D 거리 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector DistanceBTWActorsKey;

	/** AI 전방 기준 타겟 방향 각도 -180~180 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector AngleBTWActorsKey;
	
	void UpdateBlackboard(UBehaviorTreeComponent& OwnerComp);
};
