// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutEnemyHUDWidget.generated.h"

class UBlackoutEnemyHUDWidgetController;
class UProgressBar;
class UTextBlock;
/**
 * 
 */

UCLASS()
class PROJECTBLACKOUT_API UBlackoutEnemyHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWidgetController(UBlackoutEnemyHUDWidgetController* InWidgetController);

private:
	UFUNCTION()
	void OnEnemyActivated(const FText& EnemyName);

	UFUNCTION()
	void OnEnemyDeactivated();

	UFUNCTION()
	void OnEnemyHealthChanged(float Current, float Max);

	UPROPERTY()
	TObjectPtr<UBlackoutEnemyHUDWidgetController> WidgetController;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> EnemyHealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EnemyNameText;
	
};
