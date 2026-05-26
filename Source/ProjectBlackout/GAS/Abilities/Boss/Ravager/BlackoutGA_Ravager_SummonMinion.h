#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
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
class PROJECTBLACKOUT_API UGA_Ravager_SummonMinion : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void SetupEventListeners() override;
	
	virtual bool HasValidSettings() const override;

	UFUNCTION()
	void OnSpawnMinionNotify(FGameplayEventData Payload);

	void SetSpawnerProjectiles();

	void ThrowSingleSpawnerProjectile(const FVector& SpawnLocation, const FRotator& BaseRotation, int32 Index, int32 Total);

	void ResolveSpawnLocation(FVector& OutLocation) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;
};
