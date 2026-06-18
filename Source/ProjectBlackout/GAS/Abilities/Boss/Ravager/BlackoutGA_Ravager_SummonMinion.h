// ─── 구현 내역 ───────────────────────
//  - 조성원: 미니언 소환 GA 본체 — 랜덤 미니언 생성, Ravager 페이즈 연동
//  - 김민영: 정예 미니언 즉시 스폰, 지형/보스 충돌 끼임 방지, 스폰 GCN 호출 캡슐화
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_SummonMinion.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_SummonMinion : public UBlackoutGA_Ravager_Base
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

	void SpawnEliteMinionsDirectly();

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;
};
