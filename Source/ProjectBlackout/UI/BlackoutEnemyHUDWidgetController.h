// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BlackoutEnemyHUDWidgetController.generated.h"

/**
 * 
 */

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutEnemyHUDWidgetController : public UObject
{
	GENERATED_BODY()

public:
	void BindToEnemy(UAbilitySystemComponent* InEnemyASC, const FText& InEnemyName);
	void UnBindFromEnemy();
	bool IsBound() const {	return EnemyASC.IsValid(); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyActivated, const FText&, EnemyName);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeactivated);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChanged, float, Current, float, Max);

	UPROPERTY(BlueprintAssignable)
	FOnEnemyActivated OnEnemyActivated;

	UPROPERTY(BlueprintAssignable)
	FOnEnemyDeactivated OnEnemyDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnEnemyHealthChanged OnEnemyHealthChanged;

private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void BroadcastHealth();
	
	TWeakObjectPtr<UAbilitySystemComponent> EnemyASC;
	FText EnemyName;
};
