// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/BTS_CheckChaseDistance.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Engine/Engine.h"

UBTS_CheckChaseDistance::UBTS_CheckChaseDistance()
{
	NodeName = "CheckChaseDistance";
	
	Interval = 0.1f;

	bCreateNodeInstance = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;
}

void UBTS_CheckChaseDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	float Distance = BB->GetValue<UBlackboardKeyType_Float>(DistanceBTWActors.GetSelectedKeyID());
	
	BB->SetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID(), Threshold >= Distance);
}

void UBTS_CheckChaseDistance::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (auto BBAsset = GetBlackboardAsset())
	{
		DistanceBTWActors.ResolveSelectedKey(*BBAsset);
		bOnTrigger.ResolveSelectedKey(*BBAsset);
	}
}

void UBTS_CheckChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BB = OwnerComp.GetBlackboardComponent();
	BB->SetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID(), false);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
