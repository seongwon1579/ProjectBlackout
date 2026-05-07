#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SpawnMinionWave.generated.h"

/**
 * 임시 미니언 웨이브 종류 정의.
 * 프로젝트 사양에 맞게 조정 예정.
 */
UENUM(BlueprintType)
enum class EBOMinionWaveType : uint8
{
	RootHollowOnly   UMETA(DisplayName = "Root Hollow Only"),
	MixedWave        UMETA(DisplayName = "Mixed Wave (Hollow & Wraith)")
};

/**
 * UBlackoutPoolSubsystem을 사용하여 미니언 웨이브를 소환하는 Task 노드.
 * (Ravager Phase B, C 패턴 전용)
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_SpawnMinionWave : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SpawnMinionWave();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackout|Spawner")
	EBOMinionWaveType WaveType;

	UPROPERTY(EditAnywhere, Category = "Blackout|Spawner", meta = (ClampMin = 1))
	int32 SpawnCount = 3;
};
