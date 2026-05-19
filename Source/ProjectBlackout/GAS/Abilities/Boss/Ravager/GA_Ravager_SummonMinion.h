#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "BehaviorTree/Tasks/BTTask_SpawnMinionWave.h"
#include "GA_Ravager_SummonMinion.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 미니언 소환 GA.
 * WaveType 으로 Phase A(Root Hollow 단독) / Phase B 이상(혼합) 를 분기한다.
 * - Phase A: WaveType = RootHollowOnly
 * - Phase B 이상: WaveType = MixedWave (Root Hollow + Root Wraith 혼합)
 * 실제 스폰은 UBlackoutPoolSubsystem 경유 — SpawnActor 직접 호출 금지.
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_SummonMinion : public UBlackoutBossGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void SetupEventListeners() override;

	UFUNCTION()
	void OnSpawnMinionNotify(FGameplayEventData Payload);

	void SetSpawnerProjectiles();

	void ThrowSingleSpawnerProjectile(const FVector& SpawnLocation, const FRotator& BaseRotation, int32 Index, int32 Total);

	void ResolveSpawnLocation(FVector& OutLocation) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;
};
