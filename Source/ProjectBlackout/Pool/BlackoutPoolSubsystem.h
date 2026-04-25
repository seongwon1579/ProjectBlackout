#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BlackoutPoolSubsystem.generated.h"

class IBlackoutPoolableInterface;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 풀에서 액터를 꺼내거나 없으면 새로 스폰
	UFUNCTION(BlueprintCallable, Category = "Blackout|Pool")
	AActor* SpawnFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	// 액터를 풀로 반환
	UFUNCTION(BlueprintCallable, Category = "Blackout|Pool")
	void ReturnToPool(AActor* Actor);

	// 특정 클래스의 풀을 미리 워밍업
	UFUNCTION(BlueprintCallable, Category = "Blackout|Pool")
	void WarmUp(TSubclassOf<AActor> ActorClass, int32 Count);

private:
	// 클래스별 비활성 액터 풀
	TMap<TSubclassOf<AActor>, TArray<TObjectPtr<AActor>>> InactivePool;

	AActor* SpawnNewActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
};
