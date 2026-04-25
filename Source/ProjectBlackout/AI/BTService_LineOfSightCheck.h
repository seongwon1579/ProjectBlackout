#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_LineOfSightCheck.generated.h"

/**
 * 지정된 인터벌마다 타겟과의 Line Of Sight(시야)를 체크하고 Blackboard의 Boolean 키를 갱신하는 서비스 노드.
 * (Shrewd 발판 페이즈 점멸용)
 */
UCLASS()
class PROJECTBLACKOUT_API UBTService_LineOfSightCheck : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_LineOfSightCheck();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector OutHasLineOfSightKey;
};
