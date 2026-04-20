#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BlackoutBaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)           \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 플레이어 / 적 / 보스 공통 기본 어트리뷰트.
 * 플레이어 전용 스탯(Stamina, 탄약 등)은 별도 AttributeSet으로 분리.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBlackoutBaseAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ── Attributes ───────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBlackoutBaseAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBlackoutBaseAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UBlackoutBaseAttributeSet, MovementSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_BaseDamage)
	FGameplayAttributeData BaseDamage;
	ATTRIBUTE_ACCESSORS(UBlackoutBaseAttributeSet, BaseDamage)

	/** 피해 감소율 [0, 1]. 최종 피해 = 원본 × (1 - DamageReduction). */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Attributes", ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UBlackoutBaseAttributeSet, DamageReduction)

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);
	UFUNCTION()
	virtual void OnRep_BaseDamage(const FGameplayAttributeData& OldBaseDamage);
	UFUNCTION()
	virtual void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);
};
