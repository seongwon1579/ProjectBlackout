#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UBlackoutAmmoAttributeSet::UBlackoutAmmoAttributeSet()
{
}

void UBlackoutAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, PrimaryClipAmmo,      COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION(       UBlackoutAmmoAttributeSet, PrimaryMaxClip,       COND_OwnerOnly);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo,   COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, SecondaryClipAmmo,    COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION(       UBlackoutAmmoAttributeSet, SecondaryMaxClip,     COND_OwnerOnly);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, SecondaryReserveAmmo, COND_OwnerOnly, REPNOTIFY_Always);
}

void UBlackoutAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetPrimaryClipAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetPrimaryMaxClip());
	}
	else if (Attribute == GetPrimaryReserveAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetSecondaryClipAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetSecondaryMaxClip());
	}
	else if (Attribute == GetSecondaryReserveAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UBlackoutAmmoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 이펙트 적용 후 커스텀 로직 구현
}

void UBlackoutAmmoAttributeSet::OnRep_PrimaryClipAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, PrimaryClipAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_PrimaryReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_SecondaryClipAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, SecondaryClipAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_SecondaryReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, SecondaryReserveAmmo, OldValue);
}
