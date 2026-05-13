// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/BTS_UpdateAngleToTarget.h"

#include "BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTS_UpdateAngleToTarget::UBTS_UpdateAngleToTarget()
{
	bCreateNodeInstance = true;
	
	bNotifyTick = true;
	Interval = 0.1f;
}

void UBTS_UpdateAngleToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (auto BBAsset = GetBlackboardAsset())
	{
		SignedAngleKey.ResolveSelectedKey(*BBAsset);
		TargetKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTS_UpdateAngleToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	auto Owner = UBTNodeHelper::GetAIPawn(OwnerComp);
	if (!Owner) return;
	
	auto Target = Cast<APawn>(BB->GetValue<UBlackboardKeyType_Object>(TargetKey.GetSelectedKeyID()));
	if (!Target) return;
	
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
	
	const FVector Forward = Owner->GetActorForwardVector();
	
	// 앞 뒤 계산
	const float Dot = FVector::DotProduct(Forward, ToTarget);
	
	// 좌 우 계산
	const float Cross = FVector::CrossProduct(Forward, ToTarget).Z;
	
	// 각도 계산
	const float SignedAngle = FMath::RadiansToDegrees(FMath::Atan2(Cross, Dot));
	BB->SetValue<UBlackboardKeyType_Float>(SignedAngleKey.GetSelectedKeyID(), SignedAngle);
}
