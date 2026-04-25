#include "AI/BTTask_SpawnMinionWave.h"
#include "AIController.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "Engine/World.h"

UBTTask_SpawnMinionWave::UBTTask_SpawnMinionWave()
{
	NodeName = "Spawn Minion Wave";
}

EBTNodeResult::Type UBTTask_SpawnMinionWave::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UWorld* World = AIController->GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	UBlackoutPoolSubsystem* PoolSubsystem = World->GetSubsystem<UBlackoutPoolSubsystem>();
	if (!PoolSubsystem)
	{
		return EBTNodeResult::Failed;
	}

	// TODO: PoolSubsystem->AcquireFromPool을 통해 WaveType 및 SpawnCount에 맞춰 미니언 소환 처리

	return EBTNodeResult::Succeeded;
}

FString UBTTask_SpawnMinionWave::GetStaticDescription() const
{
	FString WaveTypeStr = WaveType == EBOMinionWaveType::RootHollowOnly ? TEXT("Root Hollow Only") : TEXT("Mixed Wave");
	return FString::Printf(TEXT("Spawn %d Minions (%s)"), SpawnCount, *WaveTypeStr);
}
