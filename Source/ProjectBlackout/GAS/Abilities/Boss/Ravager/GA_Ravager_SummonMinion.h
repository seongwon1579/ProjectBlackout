#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutBossGameplayAbility.h"
#include "BehaviorTree/Tasks/BTTask_SpawnMinionWave.h"
#include "GA_Ravager_SummonMinion.generated.h"

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

public:
	UGA_Ravager_SummonMinion();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 소환할 웨이브 종류. BT 에디터에서 페이즈별 노드마다 다르게 설정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	EBOMinionWaveType WaveType = EBOMinionWaveType::RootHollowOnly;

	/** 한 번에 소환할 미니언 수. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability", meta = (ClampMin = 1))
	int32 SpawnCount = 3;
};
