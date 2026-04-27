// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/GA_BossTest.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_BossTest::UGA_BossTest()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy.Attack1")));
}

void UGA_BossTest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("ActivateAbility"));
}

void UGA_BossTest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossTest::OnMontageEnded()
{
	UE_LOG(LogTemp, Warning, TEXT("MontageEnded"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
