// 제작일 : 04-21
// 제작자 : 허혁
// 수정일 : 04-21
// 수정자 :

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BlackoutAttributeMacros.h"
#include "BlackoutPlayerAttributeSet.generated.h"

/**
 * 플레이어 스탯 전용 어트리뷰트셋 
 */
	
UCLASS()
class PROJECTBLACKOUT_API UBlackoutPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UBlackoutPlayerAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_StunGauge)
	FGameplayAttributeData StunGauge;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, StunGauge)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_MaxStunGauge)
	FGameplayAttributeData MaxStunGauge;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, MaxStunGauge)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_CriticalHitChance)
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, CriticalHitChance)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_CriticalHitMultiplier)
	FGameplayAttributeData CriticalHitMultiplier;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, CriticalHitMultiplier)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_HealingEffectiveness)
	FGameplayAttributeData HealingEffectiveness;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, HealingEffectiveness)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_RelicCharges)
	FGameplayAttributeData RelicCharges;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, RelicCharges)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_MaxRelicCharges)
	FGameplayAttributeData MaxRelicCharges;
	ATTRIBUTE_ACCESSORS(UBlackoutPlayerAttributeSet, MaxRelicCharges)

protected:
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_StunGauge(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxStunGauge(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalHitChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_HealingEffectiveness(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_RelicCharges(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxRelicCharges(const FGameplayAttributeData& OldValue);
	
	
	
};
