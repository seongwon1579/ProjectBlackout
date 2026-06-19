// 제작일 : 04-21
// 제작자 : 허혁
// 수정일 : 04-21
// 수정자 :


#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "Framework/BlackoutPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UBlackoutPlayerAttributeSet::UBlackoutPlayerAttributeSet()
{
	// 유물은 모든 병과 공통으로 기본/최대 3회 고정입니다.
	InitMaxRelicCharges(3.0f);
	InitRelicCharges(3.0f);
	InitMaxStunGauge(100.0f);
	InitStunGauge(0.0f);
}

void UBlackoutPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, Stamina,               COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, MaxStamina,            COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, StunGauge,             COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, MaxStunGauge,          COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, CriticalHitChance,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, CriticalHitMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, HealingEffectiveness,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, RelicCharges,          COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutPlayerAttributeSet, MaxRelicCharges,       COND_None, REPNOTIFY_Always);
}

void UBlackoutPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStunGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStunGauge());
	}
	else if (Attribute == GetMaxStunGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
	else if (Attribute == GetCriticalHitChanceAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetCriticalHitMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetHealingEffectivenessAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetRelicChargesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxRelicCharges());
	}
	else if (Attribute == GetMaxRelicChargesAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UBlackoutPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const ABlackoutPlayerState* OwningPlayerState = Cast<ABlackoutPlayerState>(GetOwningActor());

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		const float TargetStamina = (OwningPlayerState && OwningPlayerState->HasInfiniteStaminaCheat())
			? GetMaxStamina()
			: FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina());
		SetStamina(TargetStamina);
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		SetMaxStamina(FMath::Max(GetMaxStamina(), 0.0f));
		SetStamina((OwningPlayerState && OwningPlayerState->HasInfiniteStaminaCheat())
			? GetMaxStamina()
			: FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetStunGaugeAttribute())
	{
		SetStunGauge(FMath::Clamp(GetStunGauge(), 0.0f, GetMaxStunGauge()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStunGaugeAttribute())
	{
		SetMaxStunGauge(FMath::Clamp(GetMaxStunGauge(), 0.0f, 100.0f));
		SetStunGauge(FMath::Clamp(GetStunGauge(), 0.0f, GetMaxStunGauge()));
	}
	else if (Data.EvaluatedData.Attribute == GetCriticalHitChanceAttribute())
	{
		SetCriticalHitChance(FMath::Max(GetCriticalHitChance(), 0.0f));
	}
	else if (Data.EvaluatedData.Attribute == GetCriticalHitMultiplierAttribute())
	{
		SetCriticalHitMultiplier(FMath::Max(GetCriticalHitMultiplier(), 0.0f));
	}
	else if (Data.EvaluatedData.Attribute == GetHealingEffectivenessAttribute())
	{
		SetHealingEffectiveness(FMath::Max(GetHealingEffectiveness(), 0.0f));
	}
	else if (Data.EvaluatedData.Attribute == GetRelicChargesAttribute())
	{
		SetRelicCharges(FMath::Clamp(GetRelicCharges(), 0.0f, GetMaxRelicCharges()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxRelicChargesAttribute())
	{
		SetMaxRelicCharges(FMath::Max(GetMaxRelicCharges(), 0.0f));
		SetRelicCharges(FMath::Clamp(GetRelicCharges(), 0.0f, GetMaxRelicCharges()));
	}
}

void UBlackoutPlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, Stamina, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, MaxStamina, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_StunGauge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, StunGauge, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_MaxStunGauge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, MaxStunGauge, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, CriticalHitChance, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, CriticalHitMultiplier, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_HealingEffectiveness(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, HealingEffectiveness, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_RelicCharges(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, RelicCharges, OldValue);
}

void UBlackoutPlayerAttributeSet::OnRep_MaxRelicCharges(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutPlayerAttributeSet, MaxRelicCharges, OldValue);
}
