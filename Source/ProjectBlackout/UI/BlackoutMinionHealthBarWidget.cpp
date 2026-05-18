#include "UI/BlackoutMinionHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Core/BlackoutLog.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"

void UBlackoutMinionHealthBarWidget::NativeDestruct()
{
	UnbindFromAbilitySystem();

	Super::NativeDestruct();
}

void UBlackoutMinionHealthBarWidget::BindToAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent)
{
	UnbindFromAbilitySystem();

	if (!InAbilitySystemComponent)
	{
		BO_LOG_CORE(Warning, "미니언 체력바 바인딩 실패: ASC가 유효하지 않습니다.");
		SetForceHidden(true);
		return;
	}

	BoundAbilitySystemComponent = InAbilitySystemComponent;
	bForceHidden = false;

	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UBlackoutMinionHealthBarWidget::HandleHealthChanged);
	InAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UBlackoutMinionHealthBarWidget::HandleMaxHealthChanged);

	RefreshHealth();
}

void UBlackoutMinionHealthBarWidget::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = BoundAbilitySystemComponent.Get())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
			.RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
			.RemoveAll(this);
	}

	BoundAbilitySystemComponent.Reset();
	SetForceHidden(true);
}

void UBlackoutMinionHealthBarWidget::RefreshHealth()
{
	const UAbilitySystemComponent* AbilitySystemComponent = BoundAbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		ApplyHealth(0.0f, 0.0f);
		return;
	}

	const float NewCurrentHealth =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float NewMaxHealth =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());

	ApplyHealth(NewCurrentHealth, NewMaxHealth);
}

void UBlackoutMinionHealthBarWidget::SetForceHidden(bool bInForceHidden)
{
	if (bForceHidden == bInForceHidden)
	{
		UpdateVisibilityRule();
		return;
	}

	bForceHidden = bInForceHidden;
	UpdateVisibilityRule();
}

void UBlackoutMinionHealthBarWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshHealth();
}

void UBlackoutMinionHealthBarWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshHealth();
}

void UBlackoutMinionHealthBarWidget::ApplyHealth(float NewCurrentHealth, float NewMaxHealth)
{
	MaxHealth = FMath::Max(NewMaxHealth, 0.0f);
	CurrentHealth = MaxHealth > 0.0f
		? FMath::Clamp(NewCurrentHealth, 0.0f, MaxHealth)
		: 0.0f;
	NormalizedHealth = MaxHealth > 0.0f
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(NormalizedHealth);
	}

	ReceiveHealthChanged(CurrentHealth, MaxHealth, NormalizedHealth);
	UpdateVisibilityRule();
}

void UBlackoutMinionHealthBarWidget::UpdateVisibilityRule()
{
	const bool bNewShouldShowHealthBar =
		!bForceHidden
		&& MaxHealth > 0.0f
		&& CurrentHealth > KINDA_SMALL_NUMBER
		&& CurrentHealth < MaxHealth - KINDA_SMALL_NUMBER;

	if (bShouldShowHealthBar != bNewShouldShowHealthBar)
	{
		bShouldShowHealthBar = bNewShouldShowHealthBar;
		ReceiveVisibilityRuleChanged(bShouldShowHealthBar);
	}
	else
	{
		bShouldShowHealthBar = bNewShouldShowHealthBar;
	}

	SetVisibility(bShouldShowHealthBar ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
