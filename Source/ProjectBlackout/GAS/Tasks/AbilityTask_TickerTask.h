// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 차징 공격용 AbilityTask — 매 틱 OnTick 델리게이트로 DeltaTime 전달
// ──────────────────────────────────────

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
