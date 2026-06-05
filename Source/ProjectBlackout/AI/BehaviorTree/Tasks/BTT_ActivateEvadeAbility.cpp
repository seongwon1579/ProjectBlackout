// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/BTT_ActivateEvadeAbility.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Enum/EvadeDirection.h"


UBTT_ActivateEvadeAbility::UBTT_ActivateEvadeAbility()
{
	NodeName = TEXT("Activate Evade Ability");
	bUseAbilityTagKey = false;
}

FGameplayTag UBTT_ActivateEvadeAbility::ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return FGameplayTag::EmptyTag;

	const auto Dir = static_cast<EEvadeDirection>(BB->GetValueAsEnum(EvadeDirectionKey.SelectedKeyName));
	return Dir == EEvadeDirection::Right ? RightAbilityTag : LeftAbilityTag;
}

FString UBTT_ActivateEvadeAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Evade GA: %s"), *EvadeDirectionKey.SelectedKeyName.ToString());
}
