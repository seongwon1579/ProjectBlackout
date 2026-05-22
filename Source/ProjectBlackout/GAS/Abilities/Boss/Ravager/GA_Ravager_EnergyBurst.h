#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Base.h"
#include "GA_Ravager_EnergyBurst.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * Phase B — 제자리 웅크려 충전 후 광역 에너지 파동
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_EnergyBurst : public UGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void SetupEventListeners() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnEnergyBurstNotify(FGameplayEventData Payload);
	
	UFUNCTION()
	void OffEnergyBurstNotify(FGameplayEventData Payload);

	void ApplyDamage();
	
	virtual bool IsValid() const override;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitBeginEvent;
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEndEvent;
	
	FTimerHandle DamageTimer;
};
