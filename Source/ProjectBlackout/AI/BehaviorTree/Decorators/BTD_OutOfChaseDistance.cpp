// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_OutOfChaseDistance.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Engine/Engine.h"

UBTD_OutOfChaseDistance::UBTD_OutOfChaseDistance()
{
	NodeName = TEXT("OutOfChaseDistance");
	
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTD_OutOfChaseDistance::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (auto BB = OwnerComp.GetBlackboardComponent())
	{
		auto KeyID = OnTrigger.GetSelectedKeyID();

		BB->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(
			this, &UBTD_OutOfChaseDistance::OnBlackboardKeyChanged));
	}
}

void UBTD_OutOfChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	
	if (auto BB = OwnerComp.GetBlackboardComponent())
	{
		BB->UnregisterObserversFrom(this);
	}
}

bool UBTD_OutOfChaseDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;
	
	return BB->GetValue<UBlackboardKeyType_Bool>(OnTrigger.GetSelectedKeyID());
}

void UBTD_OutOfChaseDistance::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	    
	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		OnTrigger.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTD_OutOfChaseDistance::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}

EBlackboardNotificationResult UBTD_OutOfChaseDistance::OnBlackboardKeyChanged(const UBlackboardComponent& BB,
	FBlackboard::FKey KeyID)
{
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BB.GetBrainComponent());
	if (BTComp)
	{
		BTComp->RequestExecution(this);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}
