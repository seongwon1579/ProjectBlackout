// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_FireTwinArrows.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UGA_Wraith_FireTwinArrows::UGA_Wraith_FireTwinArrows()
{
	AbilityTags.AddTag(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows);
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

	// AnimInstance onPlayMotageNotifyBegin 바인딩 - Fire1 / Fire2 NotifyName 확인
	if (ACharacter* AvatarCharacter = Cast<ACharacter>(
		GetAvatarActorFromActorInfo()))
	{
		if (USkeletalMeshComponent* MeshComp = AvatarCharacter->GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(
					this, &UGA_Wraith_FireTwinArrows::OnNotifyBegin);
			}
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BowshotMontage);
	
	if (!MontageTask)
	{
		UnbindNotify();
		EndAbility(Handle , ActorInfo, ActivationInfo , true,false);
		return;
	}
	
	MontageTask->OnCompleted.AddDynamic(this,&UGA_Wraith_FireTwinArrows::OnMontageEnded );
	MontageTask->OnBlendOut.AddDynamic(this,  &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this,  &UGA_Wraith_FireTwinArrows::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGA_Wraith_FireTwinArrows::OnNotifyBegin(FName NotifyName,
                                              const FBranchingPointNotifyPayload
                                              & Payload)
{
	if (NotifyName == TEXT("Fire1") || NotifyName == TEXT("Fire2"))
	{
		FireOneArrow();
	}
}

void UGA_Wraith_FireTwinArrows::OnMontageEnded()
{
	UnbindNotify();
	EndAbility(CurrentSpecHandle , CurrentActorInfo, CurrentActivationInfo , true,false);
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
	UBlackoutPoolSubsystem* Pool =  World->GetSubsystem<UBlackoutPoolSubsystem>();
	if (!Pool)
	{
		return;
	}
	// 발사 방향
	const FRotator AimRotation = Avatar->GetControlRotation();
	const FVector Forward = AimRotation.Vector();
	const FVector SpawnLocation = Avatar->GetActorLocation()+ Forward  *100.0f + FVector(0.0f,0.0f,50.0f);
	const FTransform SpawnTransform(AimRotation, SpawnLocation);
	
	ABOProjectile* Arrow = Cast<ABOProjectile>(Pool->SpawnFromPool(ArrowProjectileClass,SpawnTransform));
	if (!Arrow)
	{
		return;
	}
	Arrow->SetOwner(Avatar);
	Arrow->SetInstigator(Avatar);
	
	// GE Spec
	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data ->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage , DamageMagnitude);
			Arrow->InitFromSpec(SpecHandle , 0.0f);
		}
	}
	
	// 발사 Cue - VFX / SFX 
	if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SpawnLocation;
		CueParams.Normal = Forward;
		SourceASC->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Wraith_Fire , CueParams);
	}
	
	Arrow->Launch(Forward);
}

void UGA_Wraith_FireTwinArrows::UnbindNotify()
{
	if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (USkeletalMeshComponent* MeshComp = AvatarCharacter->GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGA_Wraith_FireTwinArrows::OnNotifyBegin);
			}
		}
	}
}
