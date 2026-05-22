#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Base.h"
#include "GA_Ravager_Shockwave.generated.h"

/**
 * Phase A — 앞발 충전 후 지면 장풍
 */
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTBLACKOUT_API UGA_Ravager_Shockwave : public UGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayAbilityActivationInfo ActivationInfo,
	                                const FGameplayEventData* TriggerEventData) override;
	virtual void SetupEventListeners() override;

	UFUNCTION()
	void OnSpawnProjectileNotify(FGameplayEventData Payload);

	void ResolveSpawnTransform(FVector& OutLocation, FRotator& OutRotation) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;
};
