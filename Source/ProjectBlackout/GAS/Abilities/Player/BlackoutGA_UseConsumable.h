#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_UseConsumable.generated.h"

class UBOConsumableData;

/**
 * 소모품 사용 공통 어빌리티.
 * AbilitySpec.SourceObject 또는 ConsumableData 프로퍼티로 전달된 UBOConsumableData를 읽어 수량 차감과 GE 적용을 처리합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBlackoutGA_UseConsumable : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_UseConsumable();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** SourceObject가 비어 있을 때 사용할 폴백 소모품 데이터입니다. BP GA에서 직접 지정할 수 있습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable")
	TObjectPtr<UBOConsumableData> ConsumableData;

	/** 1회 사용 시 차감할 수량입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable", meta = (ClampMin = 1))
	int32 ConsumeAmount = 1;

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
};
