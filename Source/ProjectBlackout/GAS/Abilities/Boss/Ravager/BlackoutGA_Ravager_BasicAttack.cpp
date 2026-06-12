// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_BasicAttack.h"

#include "AbilitySystemComponent.h"

namespace
{
	static const TArray<FName> EmptyHitboxNames;
}

void UBlackoutGA_Ravager_BasicAttack::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::PreActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

TSubclassOf<UGameplayEffect> UBlackoutGA_Ravager_BasicAttack::GetDamageEffect() const
{
	return CachedPatternData ? CachedPatternData->BasicAttackSettings.Effect : nullptr;
}

float UBlackoutGA_Ravager_BasicAttack::GetDamageMagnitude() const
{
	return CachedPatternData ? CachedPatternData->BasicAttackSettings.DamageMagnitude : 0.f;
}

float UBlackoutGA_Ravager_BasicAttack::GetStunMagnitude() const
{
	return CachedPatternData ? CachedPatternData->BasicAttackSettings.StunMagnitude : 0.f;
}

const TArray<FName>& UBlackoutGA_Ravager_BasicAttack::GetHitboxComponentNames() const
{
	if (!CachedPatternData) return EmptyHitboxNames;
	return CachedPatternData->BasicAttackSettings.HitboxComponentNames;
}

bool UBlackoutGA_Ravager_BasicAttack::HasValidSettings() const
{
	return CachedPatternData->BasicAttackSettings.IsValid();
}

