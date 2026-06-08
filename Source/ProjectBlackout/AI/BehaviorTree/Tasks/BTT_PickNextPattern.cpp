// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/BTT_PickNextPattern.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTT_PickNextPattern::UBTT_PickNextPattern()
{
	NodeName = TEXT("Pick Next Pattern");
}

EBTNodeResult::Type UBTT_PickNextPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB || PatternNumberKey.SelectedKeyName.IsNone()) return EBTNodeResult::Failed;
	
	const int32 Pick = FMath::RandRange(0, PatternNumber);  
	BB->SetValueAsInt(PatternNumberKey.SelectedKeyName, Pick - 1);

	return EBTNodeResult::Succeeded;
}

FString UBTT_PickNextPattern::GetStaticDescription() const
{
	return FString::Printf(TEXT("Pick random [0~%d]"), PatternNumber - 1);
}
