#include "Characters/BlackoutDamageTestDummy.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Core/BlackoutLog.h"
#include "Engine/Engine.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"

void ABlackoutDamageTestDummy::BeginPlay()
{
	Super::BeginPlay();
	ResetDummyHealth();
}

void ABlackoutDamageTestDummy::ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName)
{
	const float HealthBefore = GetCurrentHealth();

	Super::ReceiveDamageFromHitbox(SpecHandle, BoneName);

	const float HealthAfter = GetCurrentHealth();
	const float DamageTaken = FMath::Max(HealthBefore - HealthAfter, 0.0f);

	BO_LOG_GAS(Log,
	           "DamageTestDummy hit: Dummy=%s Bone=%s Damage=%.1f Health=%.1f/%.1f",
	           *GetNameSafe(this),
	           *BoneName.ToString(),
	           DamageTaken,
	           HealthAfter,
	           TestMaxHealth);

	if (bPrintDamageToScreen)
	{
		BO_SCREEN_CORE(Log,
		               "Damage Dummy: %s Damage=%.1f Health=%.1f/%.1f",
		               *GetNameSafe(this),
		               DamageTaken,
		               HealthAfter,
		               TestMaxHealth);
	}

	if (HealthBefore > 0.0f && HealthAfter <= 0.0f)
	{
		OnDeath();
	}
}

void ABlackoutDamageTestDummy::ResetDummyHealth()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetMaxHealthAttribute(), TestMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetHealthAttribute(), TestMaxHealth);

	BO_LOG_GAS(Log, "DamageTestDummy reset: Dummy=%s Health=%.1f", *GetNameSafe(this), TestMaxHealth);
}

float ABlackoutDamageTestDummy::GetCurrentHealth() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute()) : 0.0f;
}

bool ABlackoutDamageTestDummy::ShouldUseMinionHealthBar() const
{
	return true;
}

void ABlackoutDamageTestDummy::OnDeath()
{
	Super::OnDeath();

	if (bPrintDamageToScreen)
	{
		BO_SCREEN_CORE(Warning, "Damage Dummy Down: %s", *GetNameSafe(this));
	}
}
