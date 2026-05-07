#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_EvaluateAIState.generated.h"

/**
 * Distance > Approach + Approach * Threshold 여부를 평가해
 * BB의 AIState 키에 EAIState(OutOfRange / Chase)를 기록하는 서비스.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTService_EvaluateAIState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_EvaluateAIState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 현재 AI~타겟 거리 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector CurrentDistanceKey;

	/** 기준 접근 거리 (Float) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector ApproachDistanceKey;

	/** 결과를 쓸 AI 상태 키 (Enum – EAIState) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector AIStateKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector bChaseKey;

	/** 허용 편차 비율 (0.1 = 10%) */
	UPROPERTY(EditAnywhere, Category = "Blackout|Config", meta = (ClampMin = "0.0"))
	float Threshold = 0.1f;

	/** EAI_State BP 열거형에서 Chase에 해당하는 인덱스 */
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Config")
	uint8 ChaseStateIndex = 0;

	/** EAI_State BP 열거형에서 OutOfRange에 해당하는 인덱스 */
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Config")
	uint8 OutOfRangeStateIndex = 1;
	
	
};
