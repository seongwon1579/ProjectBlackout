// ─── 구현 내역 ───────────────────────
//  - 조성원: 타겟까지의 거리를 주기적으로 블랙보드에 갱신하는 서비스 (변경 감지 기반 최소 갱신)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_UpdateTargetData.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UBTS_UpdateTargetData : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_UpdateTargetData();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector TargetKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector DistanceBTWActorsKey;
	
};
