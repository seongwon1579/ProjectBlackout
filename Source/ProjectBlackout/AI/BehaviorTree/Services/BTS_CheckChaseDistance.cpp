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
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;
}

void UBTS_CheckChaseDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!IsActivation(OwnerComp)) return;

	auto BB = OwnerComp.GetBlackboardComponent();

	//float Distance = BB->GetValue<UBlackboardKeyType_Float>(DistanceBTWActors.GetSelectedKeyID());
	float Distance = BB->GetValueAsFloat(DistanceBTWActors.SelectedKeyName);

	if (Threshold < Distance)
	{
		//BB->SetValue<UBlackboardKeyType_Bool>(OnTrigger.GetSelectedKeyID(), true);
		BB->SetValueAsBool(OnTrigger.SelectedKeyName, true);
		if (GEngine) GEngine->AddOnScreenDebugMessage(200, 2.f, FColor::Yellow,
			FString::Printf(TEXT("[BTS_CheckChaseDistance] 추격 범위 이탈 — 거리: %.1f / 임계값: %.1f → OnTrigger=TRUE"), Distance, Threshold));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(201, 0.15f, FColor::Yellow,
			FString::Printf(TEXT("[BTS_CheckChaseDistance] 추격 중 — 거리: %.1f / 임계값: %.1f"), Distance, Threshold));
	}
}

void UBTS_CheckChaseDistance::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	bOnService = false;
	//BB->SetValue<UBlackboardKeyType_Bool>(OnTrigger.GetSelectedKeyID(), false);
	BB->SetValueAsBool(OnTrigger.SelectedKeyName, false);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
		TEXT("[BTS_CheckChaseDistance] ▶ 서비스 활성화 — OnTrigger 초기화"));
}

void UBTS_CheckChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	//BB->SetValue<UBlackboardKeyType_Bool>(OnTrigger.GetSelectedKeyID(), false);
	BB->SetValueAsBool(OnTrigger.SelectedKeyName, false);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
		TEXT("[BTS_CheckChaseDistance] ■ 서비스 비활성화 — OnTrigger 초기화"));
}

bool UBTS_CheckChaseDistance::IsActivation(UBehaviorTreeComponent& OwnerComp)
{
	if (bOnService) return true;
	
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;
	
	//float Distance = BB->GetValue<UBlackboardKeyType_Float>(DistanceBTWActors.GetSelectedKeyID());
	float Distance = BB->GetValueAsFloat(DistanceBTWActors.SelectedKeyName);
	if (Distance <= Threshold)
	{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
	TEXT("서비스 시작"));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
TEXT("서비스 준비중"));
	}
	bOnService = Distance <= Threshold;
	
	return bOnService;
}
