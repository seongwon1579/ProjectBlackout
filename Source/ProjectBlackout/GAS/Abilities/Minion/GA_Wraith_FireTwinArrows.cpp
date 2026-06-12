// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_FireTwinArrows.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "Engine/World.h"


UGA_Wraith_FireTwinArrows::UGA_Wraith_FireTwinArrows()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tag;
	Tag.AddTag(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows);
	SetAssetTags(Tag);
}

void UGA_Wraith_FireTwinArrows::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !BowshotMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Shot 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitShotTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Wraith_FireTwinArrows_Shot,
			nullptr, false, true);
	if (WaitShotTask)
	{
		WaitShotTask->EventReceived.AddDynamic(this, &UGA_Wraith_FireTwinArrows::OnFireShotEvent);
		WaitShotTask->ReadyForActivation();
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BowshotMontage);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(
		this, &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(
		this, &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(
		this, &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(
		this, &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGA_Wraith_FireTwinArrows::OnFireShotEvent(FGameplayEventData Payload)
{
	FireOneArrow();
}


void UGA_Wraith_FireTwinArrows::OnMontageEnded()
{
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true,
	           false);
}

void UGA_Wraith_FireTwinArrows::FireOneArrow()
{
	if (!ArrowProjectileClass)
	{
		return;
	}

	APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!Avatar)
	{
		return;
	}

	UWorld* World = Avatar->GetWorld();
	if (!World)
	{
		return;
	}
	UBlackoutPoolSubsystem* Pool = World->GetSubsystem<
		UBlackoutPoolSubsystem>();
	if (!Pool)
	{
		return;
	}
	// 발사 방향
	const FRotator AimRotation = Avatar->GetControlRotation();
	const FVector Forward = AimRotation.Vector();
	const FVector SpawnLocation = Avatar->GetActorLocation() + Forward * 100.0f
		+ FVector(0.0f, 0.0f, 50.0f);
	const FTransform SpawnTransform(AimRotation, SpawnLocation);

	ABOProjectile* Arrow = Cast<ABOProjectile>(
		Pool->SpawnFromPool(ArrowProjectileClass, SpawnTransform));
	if (!Arrow)
	{
		return;
	}
	Arrow->SetOwner(Avatar);
	Arrow->SetInstigator(Avatar);

	// GE Spec
	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
			DamageEffectClass);
		if (SpecHandle.IsValid())
		{
			// 데미지와 스턴 누적량을 같은 피격 스펙으로 함께 전달합니다.
			SpecHandle.Data->SetSetByCallerMagnitude(
				BlackoutGameplayTags::Data_Damage, DamageMagnitude);
			SpecHandle.Data->SetSetByCallerMagnitude(
				BlackoutGameplayTags::Data_Stun, StunMagnitude);
			Arrow->InitFromSpec(SpecHandle, 0.0f);
		}
	}

	// 발사 Cue - VFX / SFX 
	if (UAbilitySystemComponent* SourceASC =
		GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SpawnLocation;
		CueParams.Normal = Forward;
		SourceASC->ExecuteGameplayCue(
			BlackoutGameplayTags::GameplayCue_Wraith_Fire, CueParams);
	}

	Arrow->Launch(Forward);
}

