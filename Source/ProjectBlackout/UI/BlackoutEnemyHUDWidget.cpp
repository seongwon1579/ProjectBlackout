// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BlackoutEnemyHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BlackoutEnemyHUDWidgetController.h"


void UBlackoutEnemyHUDWidget::SetWidgetController(UBlackoutEnemyHUDWidgetController* InWidgetController)
{
	if (!InWidgetController) return;

	WidgetController = InWidgetController;

	InWidgetController->OnEnemyActivated.AddDynamic(this, &UBlackoutEnemyHUDWidget::OnEnemyActivated);
	InWidgetController->OnEnemyDeactivated.AddDynamic(this, &UBlackoutEnemyHUDWidget::OnEnemyDeactivated);
	InWidgetController->OnEnemyHealthChanged.AddDynamic(this, &UBlackoutEnemyHUDWidget::OnEnemyHealthChanged);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UBlackoutEnemyHUDWidget::OnEnemyActivated(const FText& EnemyName)
{
	EnemyNameText->SetText(FText::FromString(TEXT("Ravager")));
	SetVisibility(ESlateVisibility::Visible);
}

void UBlackoutEnemyHUDWidget::OnEnemyDeactivated()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBlackoutEnemyHUDWidget::OnEnemyHealthChanged(float Current, float Max)
{
	const float Percent = Max > 0.f ? Current / Max : 0.f;
	EnemyHealthBar->SetPercent(Percent);
}
