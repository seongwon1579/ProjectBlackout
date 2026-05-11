#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_UseRelic.generated.h"

class UAnimMontage;
class UGameplayEffect;
struct FGameplayEventData;

/**
 * 유물 사용 어빌리티.
 * Lock-in 몽타주 동안 행동을 봉쇄하고, 적용 시점에 RelicCharges를 차감한 뒤 체력을 즉시 회복합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBlackoutGA_UseRelic : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_UseRelic();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 유물 사용 시 재생할 Lock-in 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Relic")
	TObjectPtr<UAnimMontage> RelicMontage;

	/** 기본 즉시 회복량입니다. HealingEffectiveness 어트리뷰트로 보정됩니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Relic", meta = (ClampMin = 0.0))
	float RelicHealAmount = 75.0f;

	/** 1회 사용 시 차감할 유물 충전 수입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Relic", meta = (ClampMin = 1))
	int32 RelicChargeCost = 1;

	/** 몽타주 재생 중 적용할 이동속도 배율입니다. 0이면 제자리 Lock-in입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Relic", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float RelicSpeedMultiplier = 0.0f;

	/** 회복 수치 적용 후 추가로 적용할 보조 GE입니다. Cue/태그 연출이 필요할 때 BP에서 지정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Relic")
	TSubclassOf<UGameplayEffect> RelicHealEffect;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Relic Used"), Category = "Blackout|Relic")
	void ReceiveRelicUsed(float EffectiveHealAmount, int32 RemainingRelicCharges);

private:
	void ApplyRelicEffect();

	UFUNCTION()
	void OnRelicApplyEventReceived(FGameplayEventData Payload);
	void OnRelicMontageFinished();
	void ApplyRelicHealEffect();
	void ApplySlowMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo);
	void RestoreMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo);
	void BeginWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo);
	void EndWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo);

	FTimerHandle RelicMontageTimerHandle;
	float CachedWalkSpeed = 0.0f;
	bool bRelicApplied = false;
};
