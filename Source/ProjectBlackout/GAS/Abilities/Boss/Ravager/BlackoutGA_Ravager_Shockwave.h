// ─── 구현 내역 ───────────────────────
//  - 조성원: ShockWave 어빌리티 구현
//  - 김민영: 보스 GA 네트워크 정책/설계 정합성 적용
//  - 허혁: 충격파 오브젝트 풀 프리로드
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_Shockwave.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Shockwave : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayAbilityActivationInfo ActivationInfo,
	                                const FGameplayEventData* TriggerEventData) override;
	virtual void SetupEventListeners() override;
	
	virtual bool HasValidSettings() const override;

	UFUNCTION()
	void OnSpawnProjectileNotify(FGameplayEventData Payload);

	void ResolveSpawnTransform(FVector& OutLocation, FRotator& OutRotation) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;
};
