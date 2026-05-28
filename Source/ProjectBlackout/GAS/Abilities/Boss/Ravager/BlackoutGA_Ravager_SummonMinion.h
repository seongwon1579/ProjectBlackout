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

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSpawnEvent;

	UPROPERTY(Transient)
	FTimerHandle EliteSpawnTimerHandle;
};
