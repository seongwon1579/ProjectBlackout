#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CompareDistance.generated.h"

/**
 * 블랙보드의 거리 값이 [MinDistance, MaxDistance] 범위 안에 있으면 true.
 * MaxDistance = 0 이면 상한 없음.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTDecorator_CompareDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CompareDistance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

	/** BTService_UpdateTargetData 가 쓰는 Distance 키 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector DistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Distance", meta = (ClampMin = "0.0"))
	float MinDistance = 0.f;

	/** 0 이면 상한 없음 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Distance", meta = (ClampMin = "0.0"))
	float MaxDistance = 0.f;
};