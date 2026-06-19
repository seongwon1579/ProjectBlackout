// ─── 구현 내역 ───────────────────────
//  - 김민영: 클래스별 비활성 액터 풀 기반 스폰/반환/워밍업 World 서브시스템과 GC 추적·댕글링 포인터 방어 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BlackoutPoolSubsystem.generated.h"

class IBlackoutPoolableInterface;

// TMap의 값으로 TArray를 직접 UPROPERTY 지정할 수 없어(중첩 컨테이너 미지원) 구조체로 래핑합니다.
// UPROPERTY로 노출해야 GC가 풀에 보관된 액터 참조를 추적하여, 파괴된 액터 포인터가 null로 정리되고
// 살아있는 풀 액터는 수거되지 않습니다.
USTRUCT()
struct FBlackoutPooledActorArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

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
	// 클래스별 비활성 액터 풀 (GC 추적을 위해 UPROPERTY)
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FBlackoutPooledActorArray> InactivePool;

	AActor* SpawnNewActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
};
