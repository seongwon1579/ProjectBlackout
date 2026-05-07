#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_CheckPhaseExit.generated.h"

/**
 * 어빌리티 완료 직후(= 안전 지점)에 페이즈 전환 요청을 확인하는 BT Task.
 *
 * StateTree가 BB_RequestPhaseExit 키를 true 로 설정하면,
 * 현재 어빌리티가 끝난 뒤 이 Task에서 BT를 안전하게 종료한다.
 * BT 종료 → FBSTTask_RunSubBehaviorTree::Tick 감지 → StateTree 페이즈 전환.
 *
 * 배치 위치: BT 루프 내 각 어빌리티 Task 바로 다음.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_CheckPhaseExit : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CheckPhaseExit();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** StateTree가 설정하는 페이즈 탈출 요청 블랙보드 키 (Bool). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector PhaseExitKey;
};