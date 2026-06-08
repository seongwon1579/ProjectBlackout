// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/BTT_SelectAbility.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTT_SelectAbility::UBTT_SelectAbility()
{
	NodeName = TEXT("SelectAbility");
}

EBTNodeResult::Type UBTT_SelectAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	BB->SetValueAsName(SelectedAbilityTagKey.SelectedKeyName, AbilityTagToSet.GetTagName());
	
	return EBTNodeResult::Succeeded;
}

FString UBTT_SelectAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Selected Key = %s"), *AbilityTagToSet.ToString());
}
