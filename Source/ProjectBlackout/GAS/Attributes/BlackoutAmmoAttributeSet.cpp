#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "Framework/BlackoutPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UBlackoutAmmoAttributeSet::UBlackoutAmmoAttributeSet()
{
}

void UBlackoutAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, PrimaryClipAmmo,      COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, PrimaryMaxClip,       COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo,   COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, SecondaryClipAmmo,    COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutAmmoAttributeSet, SecondaryMaxClip,     COND_OwnerOnly, REPNOTIFY_Always);
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

	const ABlackoutPlayerState* OwningPlayerState = Cast<ABlackoutPlayerState>(GetOwningActor());
	if (!OwningPlayerState || !OwningPlayerState->HasInfiniteAmmoCheat())
	{
		return;
	}

	if (Data.EvaluatedData.Attribute == GetPrimaryClipAmmoAttribute()
		|| Data.EvaluatedData.Attribute == GetPrimaryMaxClipAttribute())
	{
		SetPrimaryClipAmmo(GetPrimaryMaxClip());
	}
	else if (Data.EvaluatedData.Attribute == GetSecondaryClipAmmoAttribute()
		|| Data.EvaluatedData.Attribute == GetSecondaryMaxClipAttribute())
	{
		SetSecondaryClipAmmo(GetSecondaryMaxClip());
	}
}

void UBlackoutAmmoAttributeSet::OnRep_PrimaryClipAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, PrimaryClipAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_PrimaryMaxClip(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, PrimaryMaxClip, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_PrimaryReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_SecondaryClipAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, SecondaryClipAmmo, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_SecondaryMaxClip(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, SecondaryMaxClip, OldValue);
}

void UBlackoutAmmoAttributeSet::OnRep_SecondaryReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutAmmoAttributeSet, SecondaryReserveAmmo, OldValue);
}
