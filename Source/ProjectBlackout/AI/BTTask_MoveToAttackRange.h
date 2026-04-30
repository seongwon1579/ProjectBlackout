#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_MoveToAttackRange.generated.h"

/**
 * 타겟을 동적으로 추적하며 ApproachDistance까지 접근하는 BT Task.
 *
 * - 이미 ApproachDistance 이내이면 즉시 Succeeded.
 * - 이동 중 타겟이 MaxDistance 밖으로 벗어나면 이동을 중단하고 즉시 Succeeded.
 *   (멀리 도망쳐도 어빌리티를 실행해 무한 추격을 방지한다.)
 * - 이동 완료(경로 추종 결과 무관) 시 Succeeded.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_MoveToAttackRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToAttackRange();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void                TickTask  (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString             GetStaticDescription() const override;

protected:
	/** 추적할 타겟 액터 블랙보드 키 (Object). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;

	/** BTTask_SelectPattern이 기록한 접근 목표 거리 키 (Float). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector ApproachDistanceKey;

	/** BTTask_SelectPattern이 기록한 패턴 최대 거리 키 (Float).
	 *  이동 중 이 거리를 초과하면 즉시 Succeeded. 0이면 검사 생략. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector MaxDistanceKey;

private:
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
	void StopAndSucceed(UBehaviorTreeComponent& OwnerComp);

	UBehaviorTreeComponent*                  CachedOwnerComp = nullptr;
	TWeakObjectPtr<AAIController>            CachedAI;
	FAIRequestID                             CachedRequestID;
};
