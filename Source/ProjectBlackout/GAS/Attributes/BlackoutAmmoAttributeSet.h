#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BlackoutAttributeMacros.h"
#include "BlackoutAmmoAttributeSet.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UBlackoutAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBlackoutAmmoAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", ReplicatedUsing = OnRep_PrimaryClipAmmo)
	FGameplayAttributeData PrimaryClipAmmo;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, PrimaryClipAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", Replicated)
	FGameplayAttributeData PrimaryMaxClip;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, PrimaryMaxClip)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", ReplicatedUsing = OnRep_PrimaryReserveAmmo)
	FGameplayAttributeData PrimaryReserveAmmo;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", ReplicatedUsing = OnRep_SecondaryClipAmmo)
	FGameplayAttributeData SecondaryClipAmmo;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, SecondaryClipAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", Replicated)
	FGameplayAttributeData SecondaryMaxClip;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, SecondaryMaxClip)

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Ammo", ReplicatedUsing = OnRep_SecondaryReserveAmmo)
	FGameplayAttributeData SecondaryReserveAmmo;
	ATTRIBUTE_ACCESSORS(UBlackoutAmmoAttributeSet, SecondaryReserveAmmo)

protected:
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_PrimaryReserveAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_SecondaryReserveAmmo(const FGameplayAttributeData& OldValue);
};
