// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_IsInRange.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTD_IsInRange::UBTD_IsInRange()
{
	
}

bool UBTD_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;
	
	float Distance = BB->GetValueAsFloat(DistanceBTWActors.SelectedKeyName);
	return IsInRange(Distance, MinRange, MaxRange);
}

FString UBTD_IsInRange::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}

bool UBTD_IsInRange::IsInRange(float Distance, float Min, float Max) const
{
	const bool bHasMin = Min >= 0.f;
	const bool bHasMax = Max >= 0.f;

	if (bHasMin && bHasMax) return Distance >= Min && Distance <= Max;
	if (bHasMin)            return Distance <= Min;
	if (bHasMax)            return Distance >= Max;
	return true;
}
