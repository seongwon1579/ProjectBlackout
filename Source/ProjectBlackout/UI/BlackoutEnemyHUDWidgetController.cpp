// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BlackoutEnemyHUDWidgetController.h"

#include "AbilitySystemComponent.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutLog.h"


void UBlackoutEnemyHUDWidgetController::BindToEnemy(UAbilitySystemComponent* InEnemyASC, const FText& InEnemyName)
{
	UnBindFromEnemy();

	if (!InEnemyASC)
	{
		BO_LOG_CORE(Warning, "Enemy HUD 바인딩 실패: ASC가 유효하지 않습니다.");
		return;
	}

	EnemyASC = InEnemyASC;
	EnemyName = InEnemyName;

	InEnemyASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
	          .AddUObject(this, &UBlackoutEnemyHUDWidgetController::HandleHealthChanged);

	InEnemyASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
	          .AddUObject(this, &UBlackoutEnemyHUDWidgetController::HandleMaxHealthChanged);
	
	OnEnemyActivated.Broadcast(EnemyName);
	BroadcastHealth();
}

void UBlackoutEnemyHUDWidgetController::UnBindFromEnemy()
{
	if (UAbilitySystemComponent* ASC = EnemyASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
	}
	EnemyASC.Reset();
	EnemyName = FText::GetEmpty();
	OnEnemyDeactivated.Broadcast();
}

void UBlackoutEnemyHUDWidgetController::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	BroadcastHealth();
}

void UBlackoutEnemyHUDWidgetController::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	BroadcastHealth();
}

void UBlackoutEnemyHUDWidgetController::BroadcastHealth()
{
	const UAbilitySystemComponent* ASC = EnemyASC.Get();
	if (!ASC) return;
	
	const float Health = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	OnEnemyHealthChanged.Broadcast(Health, MaxHealth);
}
