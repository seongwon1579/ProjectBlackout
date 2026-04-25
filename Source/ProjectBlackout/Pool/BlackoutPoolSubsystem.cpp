#include "BlackoutPoolSubsystem.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutLog.h"

AActor* UBlackoutPoolSubsystem::SpawnFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	TArray<TObjectPtr<AActor>>& Pool = InactivePool.FindOrAdd(ActorClass);

	// 유효한 액터가 나올 때까지 탐색
	while (Pool.Num() > 0)
	{
		TObjectPtr<AActor> Candidate = Pool.Pop(EAllowShrinking::No);
		if (IsValid(Candidate))
		{
			Candidate->SetActorTransform(SpawnTransform);
			IBlackoutPoolableInterface::Execute_OnSpawnFromPool(Candidate);
			BO_LOG_POOL(Verbose, "SpawnFromPool(reuse): %s", *Candidate->GetName());
			return Candidate;
		}
	}

	// 풀이 비어 있으면 새로 스폰
	return SpawnNewActor(ActorClass, SpawnTransform);
}

void UBlackoutPoolSubsystem::ReturnToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	IBlackoutPoolableInterface::Execute_OnReturnToPool(Actor);
	InactivePool.FindOrAdd(Actor->GetClass()).Add(Actor);
	BO_LOG_POOL(Verbose, "ReturnToPool: %s", *Actor->GetName());
}

void UBlackoutPoolSubsystem::WarmUp(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass || Count <= 0)
	{
		return;
	}

	TArray<TObjectPtr<AActor>>& Pool = InactivePool.FindOrAdd(ActorClass);
	for (int32 i = 0; i < Count; ++i)
	{
		AActor* NewActor = SpawnNewActor(ActorClass, FTransform::Identity);
		if (NewActor)
		{
			IBlackoutPoolableInterface::Execute_OnReturnToPool(NewActor);
			Pool.Add(NewActor);
		}
	}
	BO_LOG_POOL(Log, "WarmUp: %s x%d", *ActorClass->GetName(), Count);
}

AActor* UBlackoutPoolSubsystem::SpawnNewActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, Params);
	BO_LOG_POOL(Verbose, "SpawnFromPool(new): %s", *GetNameSafe(NewActor));
	return NewActor;
}
