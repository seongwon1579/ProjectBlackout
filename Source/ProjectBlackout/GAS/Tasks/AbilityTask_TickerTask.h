// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TickerTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTickerTaskDelegate, float, DeltaTime);
/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UAbilityTask_TickerTask : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UAbilityTask_TickerTask();

	UPROPERTY(BlueprintAssignable)
	FTickerTaskDelegate OnTick;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_TickerTask* CreateTickerTask(UGameplayAbility* OwningAbility);

protected:
	virtual void TickTask(float DeltaTime) override;
};
