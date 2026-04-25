#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_TurnToTarget.generated.h"

/** ExecuteTask 중 상태 보존용 NodeMemory. */
struct FBTTask_TurnMemory
{
	UAnimMontage* ActiveMontage        = nullptr;
	bool          bWasOrientToMovement = false;
};

/**
 * 현재 바라보는 방향과 타겟 간 각도를 계산해 좌/우 회전 몽타주를 재생하는 BT Task.
 * 각도 계산은 UBOAICalcHelper::GetTurnDirection() 에 위임한다.
 * 몽타주가 끝나면 Succeeded, 임계값 이내면 즉시 Succeeded.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_TurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TurnToTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void                TickTask  (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16              GetInstanceMemorySize() const override { return sizeof(FBTTask_TurnMemory); }

	/** 회전 방향을 조회할 타겟 블랙보드 키. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 이 각도(도) 미만이면 회전 없이 즉시 Succeeded. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Turn", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float TurnThreshold = 45.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Turn")
	TObjectPtr<UAnimMontage> TurnLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Blackout|Turn")
	TObjectPtr<UAnimMontage> TurnRightMontage;

private:
	void RestoreMovement(ACharacter* Char, bool bOrientToMovement) const;
};