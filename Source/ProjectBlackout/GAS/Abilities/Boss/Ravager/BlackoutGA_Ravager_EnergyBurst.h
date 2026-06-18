// ─── 구현 내역 ───────────────────────
//  - 조성원: EnergyBurst 어빌리티 구현
//  - 김민영: 보스 GA 네트워크 정책/설계 정합성 적용
//  - 허혁: 적중 시 플레이어 스턴 게이지 부여
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_EnergyBurst.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * Phase B — 제자리 웅크려 충전 후 광역 에너지 파동
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_EnergyBurst : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnEnergyBurstNotify(FGameplayEventData Payload);
	
	UFUNCTION()
	void OffEnergyBurstNotify(FGameplayEventData Payload);

	void ApplyDamage();
	
	virtual bool HasValidSettings() const override;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitBeginEvent;
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEndEvent;
	
	FTimerHandle DamageTimer;
};
