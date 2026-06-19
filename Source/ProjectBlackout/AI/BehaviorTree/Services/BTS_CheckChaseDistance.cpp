// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/BTS_CheckChaseDistance.h"

#include "BlackoutBossCharacter.h"
#include "BOBossChaseRanges.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Engine/Engine.h"

UBTS_CheckChaseDistance::UBTS_CheckChaseDistance()
{
	NodeName = "CheckChaseDistance";

	Interval = 0.1f;

	bNotifyBecomeRelevant = true;
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
	bool bCurrentTrigger = BB->GetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID());
	bool bNextTrigger = bCurrentTrigger;
	
	if (bCurrentTrigger)
	{
		if (Distance > CachedRanges.ChaseEndRange)
		{
			bNextTrigger = false;
		}
	}
	else
	{
		if (Distance < CachedRanges.ChaseStartRange)
		{
			bNextTrigger = true;
		}
	}
	
	if (bNextTrigger != bCurrentTrigger)
	{
		BB->SetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID(), bNextTrigger);
	}
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

void UBTS_CheckChaseDistance::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;
	
	ABlackoutBossCharacter* Owner = Cast<ABlackoutBossCharacter>(AICon->GetPawn());
	if (!IsValid(Owner)) return;

	FGameplayTag PatternTag = FGameplayTag::RequestGameplayTag(BB->GetValueAsName(ActiveAbilityTagKey.SelectedKeyName));

	CachedRanges = Owner->GetChaseRanges(PatternTag);

	
	const float RandomMultiplier = FMath::FRandRange(1.f - CachedRanges.AttackRangeVariance, 1.f + CachedRanges.AttackRangeVariance);
	const float FinalAttackRange = CachedRanges.AttackRange * RandomMultiplier;
	BB->SetValueAsFloat(AcceptanceRadiusKey.SelectedKeyName, FinalAttackRange);
	
}

void UBTS_CheckChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID(), false);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
