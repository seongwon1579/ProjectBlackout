#include "BlackoutPoolSubsystem.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutLog.h"

AActor* UBlackoutPoolSubsystem::SpawnFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	TArray<TObjectPtr<AActor>>& Pool = InactivePool.FindOrAdd(ActorClass).Actors;

	// 유효한 액터가 나올 때까지 탐색
	while (Pool.Num() > 0)
	{
		TObjectPtr<AActor> Candidate = Pool.Pop(EAllowShrinking::No);
		if (IsValid(Candidate))
		{
			// 가시성 복원을 먼저 수행한 뒤 위치를 이동시켜야, 클라이언트가 "숨겨진 상태의 위치 이동" 패킷을
			// 먼저 받아 위젯/이펙트가 한 프레임 꺼지는 레이스를 피할 수 있습니다.
			IBlackoutPoolableInterface::Execute_OnSpawnFromPool(Candidate);
			Candidate->SetActorTransform(SpawnTransform);
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

	// 같은 액터가 한 프레임에 여러 번 반환되면(예: ProjectileMovement sub-step sweep으로 OnHit 다중 발생)
	// 풀에 동일 포인터가 중복 등록되어, 이후 서로 다른 두 발사가 같은 인스턴스를 공유하게 됩니다.
	// 이는 네트워크 액터 채널 상태 손상으로 이어지므로 중복 반환을 무시합니다.
	TArray<TObjectPtr<AActor>>& Pool = InactivePool.FindOrAdd(Actor->GetClass()).Actors;
	if (Pool.Contains(Actor))
	{
		BO_LOG_POOL(Warning, "ReturnToPool 무시: 이미 풀에 존재하는 액터의 중복 반환 (%s)", *Actor->GetName());
		return;
	}

	IBlackoutPoolableInterface::Execute_OnReturnToPool(Actor);
	Pool.Add(Actor);
	BO_LOG_POOL(Verbose, "ReturnToPool: %s", *Actor->GetName());
}

void UBlackoutPoolSubsystem::WarmUp(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass || Count <= 0)
	{
		return;
	}

	TArray<TObjectPtr<AActor>>& Pool = InactivePool.FindOrAdd(ActorClass).Actors;
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
