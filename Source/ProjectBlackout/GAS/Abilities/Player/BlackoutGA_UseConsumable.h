#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_UseConsumable.generated.h"

class UAnimMontage;
class UBOConsumableData;
struct FGameplayEventData;

/**
 * 소모품 사용 공통 어빌리티.
 * AbilitySpec.SourceObject 또는 ConsumableData 프로퍼티로 전달된 UBOConsumableData를 읽어 수량 차감과 GE 적용을 처리합니다.
 * ConsumableMontage가 설정되면 몽타주 재생 중 이동속도를 감소시키고,
 * Event.Montage.ConsumableApply AnimNotify 시점에 실제 소모와 효과를 적용합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBlackoutGA_UseConsumable : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_UseConsumable();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** SourceObject가 비어 있을 때 사용할 폴백 소모품 데이터입니다. BP GA에서 직접 지정할 수 있습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable")
	TObjectPtr<UBOConsumableData> ConsumableData;

	/** 1회 사용 시 차감할 수량입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable", meta = (ClampMin = 1))
	int32 ConsumeAmount = 1;

	/** 소모품 사용 시 재생할 애니메이션 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable")
	TObjectPtr<UAnimMontage> ConsumableMontage;

	/** 몽타주 재생 중 적용할 이동속도 배율입니다. (1.0 = 감소 없음) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float ConsumableSpeedMultiplier = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Consumable")
	const UBOConsumableData* ResolveConsumableData();

	virtual bool ApplyConsumableEffect(const UBOConsumableData* UsedConsumableData);
	virtual bool ShouldApplyConfiguredGameplayEffect(const UBOConsumableData* UsedConsumableData) const;
	void ApplyConfiguredGameplayEffect(const UBOConsumableData* UsedConsumableData);
	float GetEffectMagnitudeOrDefault(const UBOConsumableData* UsedConsumableData, FGameplayTag MagnitudeTag, float DefaultValue) const;
	bool IsConsumableCooldownReady(const UBOConsumableData* UsedConsumableData) const;
	void StartConsumableCooldown(const UBOConsumableData* UsedConsumableData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Used"), Category = "Blackout|Consumable")
	void ReceiveConsumableUsed(const UBOConsumableData* UsedConsumableData);

	float ConsumableCooldownEndTime = 0.0f;

private:
	void ConsumeAndApplyEffect();

	UFUNCTION()
	void OnConsumableApplyEventReceived(FGameplayEventData Payload);
	void OnConsumableMontageFinished();
	void ApplySlowMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo);
	void RestoreMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo);
	void BeginWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo);
	void EndWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo);

	FTimerHandle ConsumableMontageTimerHandle;
	TObjectPtr<const UBOConsumableData> PendingConsumableData;
	float CachedWalkSpeed = 0.0f;
	bool bKeepActiveAfterMontage = false;
	bool bConsumableApplied = false;
};
