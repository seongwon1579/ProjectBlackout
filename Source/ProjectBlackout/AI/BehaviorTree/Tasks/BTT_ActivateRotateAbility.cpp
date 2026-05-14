// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Tasks/BTT_ActivateRotateAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Abilities/GameplayAbility.h"


UBTT_ActivateRotateAbility::UBTT_ActivateRotateAbility()
{
	NodeName = TEXT("Activate Rotate Ability");
}

void UBTT_ActivateRotateAbility::PrepareEventData(FGameplayEventData& EventData, UBlackboardComponent* BB)
{
	EventData.EventMagnitude = BB->GetValueAsFloat(SignedAngleKey.SelectedKeyName);
}
