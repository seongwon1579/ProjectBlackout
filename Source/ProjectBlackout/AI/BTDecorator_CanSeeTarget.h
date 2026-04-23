#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanSeeTarget.generated.h"

/**
 * 분기 평가 시점에 라인 트레이스로 타겟 시야를 즉시 체크하는 데코레이터.
 * BTService_LineOfSightCheck 는 주기적 갱신용이고, 이 데코레이터는 분기 판단 시 즉시 판단용.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTDecorator_CanSeeTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanSeeTarget();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;
};