#include "BlackoutBaseAttributeSet.h"
#include "Framework/BlackoutPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "BlackoutLog.h"

UBlackoutBaseAttributeSet::UBlackoutBaseAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMovementSpeed(400.f);
	InitBaseDamage(10.f);
	InitDamageReduction(0.f);
}

void UBlackoutBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutBaseAttributeSet, Health,         COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutBaseAttributeSet, MaxHealth,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutBaseAttributeSet, MovementSpeed,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutBaseAttributeSet, BaseDamage,      COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBlackoutBaseAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
}

void UBlackoutBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// MaxHealth 변경 시 Health를 비율 유지하며 스케일
		const float OldMaxHealth = GetMaxHealth();
		if (OldMaxHealth > 0.f)
		{
			const float Ratio = GetHealth() / OldMaxHealth;
			SetHealth(NewValue * Ratio);
		}
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetDamageReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
}

void UBlackoutBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const ABlackoutPlayerState* OwningPlayerState = Cast<ABlackoutPlayerState>(GetOwningActor());

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float ClampedHealth = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		if (OwningPlayerState && OwningPlayerState->HasInfiniteHealthCheat())
		{
			ClampedHealth = GetMaxHealth();
		}

		SetHealth(ClampedHealth);

		if (ClampedHealth <= 0.f)
		{
			BO_LOG_GAS(Log, "Health reached zero: %s", *GetOwningActor()->GetName());
			// 실제 Downed/Death 분기는 ABlackoutCharacterBase::ApplyIncomingDamageSpec()에서 처리합니다.
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 0.f));
		SetHealth((OwningPlayerState && OwningPlayerState->HasInfiniteHealthCheat())
			? GetMaxHealth()
			: FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UBlackoutBaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutBaseAttributeSet, Health, OldHealth);
}

void UBlackoutBaseAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutBaseAttributeSet, MaxHealth, OldMaxHealth);
}

void UBlackoutBaseAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutBaseAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UBlackoutBaseAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldBaseDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutBaseAttributeSet, BaseDamage, OldBaseDamage);
}

void UBlackoutBaseAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBlackoutBaseAttributeSet, DamageReduction, OldDamageReduction);
}
