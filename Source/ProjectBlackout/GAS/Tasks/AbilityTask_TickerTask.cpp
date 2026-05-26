// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tasks/AbilityTask_TickerTask.h"

UAbilityTask_TickerTask::UAbilityTask_TickerTask()
{
	bTickingTask = true;
}

UAbilityTask_TickerTask* UAbilityTask_TickerTask::CreateTickerTask(UGameplayAbility* OwningAbility)
{
	return NewAbilityTask<UAbilityTask_TickerTask>(OwningAbility);
}

void UAbilityTask_TickerTask::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}