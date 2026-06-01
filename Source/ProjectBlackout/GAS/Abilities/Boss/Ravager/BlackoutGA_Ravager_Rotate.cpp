// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Rotate.h"

#include "BlackoutGameplayTags.h"
#include "BORavagerBoss.h"
#include "Abilities/Tasks/AbilityTask.h"

void UBlackoutGA_Ravager_Rotate::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

FGameplayTag UBlackoutGA_Ravager_Rotate::SelectMontageTag(const FGameplayEventData* TriggerEventData) const
{
	if (!TriggerEventData)
	{
		return FGameplayTag::EmptyTag;
	}

	const float SignedAngle = TriggerEventData->EventMagnitude;
	const float AbsAngle = FMath::Abs(SignedAngle);
	const bool bRight = SignedAngle > 0.f;

	if (AbsAngle >= To180Threshold)
	{
		return bRight
			? BlackoutGameplayTags::Ability_Ravager_Turn_R_180
			: BlackoutGameplayTags::Ability_Ravager_Turn_L_180;
	}
	if (AbsAngle >= To135Threshold)
	{
		return bRight
			? BlackoutGameplayTags::Ability_Ravager_Turn_R_135
			: BlackoutGameplayTags::Ability_Ravager_Turn_L_135;
	}
	if (AbsAngle >= To90Threshold)
	{
		return bRight
			? BlackoutGameplayTags::Ability_Ravager_Turn_R_90
			: BlackoutGameplayTags::Ability_Ravager_Turn_L_90;
	}

	return FGameplayTag::EmptyTag;
}