#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTDecorator_CheckAbilityCondition.generated.h"

/**
 * GA 실행 전 조건을 검사하는 BT Decorator.
 *
 * 검사 항목:
 *   1. 타겟 액터가 유효하고 소멸 중이 아닌가
 *   2. MaxRange > 0 이면 AI ~ 타겟 2D 거리가 MaxRange 이내인가
 *
 * 조건 불충족 시 해당 노드(또는 서브트리)를 실행하지 않는다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTDecorator_CheckAbilityCondition : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckAbilityCondition();

	virtual bool   CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 유효성을 검사할 타겟 블랙보드 키 (Object). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Condition")
	FBlackboardKeySelector TargetKey;

	/**
	 * 최대 허용 거리 (cm). 0이면 거리 검사를 생략한다.
	 * BTService_UpdateTargetData의 OutDistanceKey와 함께 쓰거나,
	 * 직접 거리를 계산한다.
	 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Condition", meta = (ClampMin = "0.0"))
	float MaxRange = 0.f;
};
