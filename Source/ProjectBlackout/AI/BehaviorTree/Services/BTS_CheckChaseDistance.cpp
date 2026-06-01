// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/BTS_CheckChaseDistance.h"

#include "BlackoutBossCharacter.h"
#include "BORavagerBoss.h"
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

	if (!IsValid(CachedData)) return;

	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	float Distance = BB->GetValue<UBlackboardKeyType_Float>(DistanceBTWActors.GetSelectedKeyID());
	bool bCurrentTrigger = BB->GetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID());
	bool bNextTrigger = bCurrentTrigger;
	
	UE_LOG(LogTemp, Warning, TEXT("Range:%f"), CachedData->AttackRange)

	if (bCurrentTrigger)
	{
		if (Distance > CachedData->ChaseEndRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("To False"))
			bNextTrigger = false;
		}
	}
	else
	{
		if (Distance < CachedData->ChaseStartRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("To True"))
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

	ABORavagerBoss* Owner = Cast<ABORavagerBoss>(AICon->GetPawn());
	if (!IsValid(Owner)) return;

	FGameplayTag PatternTag = FGameplayTag::RequestGameplayTag(BB->GetValueAsName(ActiveAbilityTagKey.SelectedKeyName));

	CachedData = nullptr;

	if (PatternTag.IsValid())
	{
		CachedData = Owner->GetPatternData(PatternTag);
	}
	
	if (IsValid(CachedData))
	{
		const float RandomMultiplier = FMath::FRandRange(1.f - CachedData->AttackRangeVariance, 1.f + CachedData->AttackRangeVariance);
		const float FinalAttackRange = CachedData->AttackRange * RandomMultiplier;
		BB->SetValueAsFloat(AcceptanceRadiusKey.SelectedKeyName, FinalAttackRange);
	}
}

void UBTS_CheckChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValue<UBlackboardKeyType_Bool>(bOnTrigger.GetSelectedKeyID(), false);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
