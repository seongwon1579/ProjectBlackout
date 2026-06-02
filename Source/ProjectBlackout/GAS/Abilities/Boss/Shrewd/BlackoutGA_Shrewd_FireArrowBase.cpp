// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_FireArrowBase.h"

#include "BlackoutGameplayTags.h"
#include "BlackoutPoolSubsystem.h"
#include "BOProjectile.h"
#include "BOShrewdBoss.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"

void UBlackoutGA_Shrewd_FireArrowBase::SetupEventListeners()
{
	UAbilityTask_WaitGameplayEvent* WaitShotTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Shrewd_Attack_FireArrow,
		nullptr,
		false,
		true);

	if (WaitShotTask)
	{
		WaitShotTask->EventReceived.AddDynamic(this, &UBlackoutGA_Shrewd_FireArrowBase::OnFireShotEvent);
		WaitShotTask->ReadyForActivation();
	}
}

void UBlackoutGA_Shrewd_FireArrowBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	
	if (ABOShrewdBoss* Boss = Cast<ABOShrewdBoss>(GetAvatarActorFromActorInfo()))
	{
		Boss->SetBowVisible(true);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Shrewd_FireArrowBase::OnFireShotEvent(FGameplayEventData Payload)
{
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Avatar || !ArrowProjectileClass) return;
	
	if (ABOShrewdBoss* Boss = Cast<ABOShrewdBoss>(GetAvatarActorFromActorInfo()))
	{
		Boss->SetBowVisible(false);
	}

	UWorld* World = GetWorld();
	AActor* Target = CachedTarget;
	if (!World || !Target) return;
	
	FVector SpawnLocation = Avatar->GetActorLocation();
	FRotator SpawnRotation = Avatar->GetActorRotation();
	 if (USkeletalMeshComponent* Mesh = Avatar->GetMesh())
	 {
	 	if (Mesh->DoesSocketExist(SpawnSocketName))
	 	{
	 		SpawnLocation = Mesh->GetSocketLocation(SpawnSocketName);
	 		SpawnRotation = Mesh->GetSocketRotation(SpawnSocketName);
	 	}
	 }

	UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>();
	if (!Pool) return;

	const FVector TargetLocation = Target->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Avatar;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	ABOProjectile* Arrow = Cast<ABOProjectile>(
		Pool->SpawnFromPool(ArrowProjectileClass, SpawnTransform));
	
	if (!Arrow) return;

	Arrow->SetOwner(Avatar);
	Arrow->SetInstigator(Avatar);
	
	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle =
			MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
			    BlackoutGameplayTags::Data_Damage, DamageMagnitude);
			Arrow->InitFromSpec(SpecHandle, 0.0f);
		}
	}

	LaunchProjectile(Arrow, SpawnLocation, TargetLocation);
}
