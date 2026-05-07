// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_RandomChance.h"

UBTD_RandomChance::UBTD_RandomChance()
{
}

bool UBTD_RandomChance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FMath::FRand() <= Random;
}

FString UBTD_RandomChance::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
