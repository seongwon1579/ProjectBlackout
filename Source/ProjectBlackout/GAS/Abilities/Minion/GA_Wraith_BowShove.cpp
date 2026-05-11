// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_BowShove.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayAnimAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/Tasks/AbilityTask_BossMeleeSweep.h"
#include "Interfaces/BlackoutDamageable.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UGA_Wraith_BowShove::UGA_Wraith_BowShove()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tag;
	Tag.AddTag(BlackoutGameplayTags::Ability_Wraith_BowShove);
	SetAssetTags(Tag);
}

void UGA_Wraith_BowShove::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !
		BowShoveAnimMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BowShoveAnimMontage);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	MontageTask->OnCompleted.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(
		this, &UGA_Wraith_BowShove::OnMontageEnded);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Enemy_Attack_SweepStart, nullptr,
			true, true
		);

	if (WaitStartTask)
	{
		WaitStartTask->EventReceived.AddDynamic(
			this, &UGA_Wraith_BowShove::OnSweepStartEvent);
		WaitStartTask->ReadyForActivation();
	}
}

void UGA_Wraith_BowShove::OnSweepStartEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Wraith BowShove: SweepStart"));

	// BowMesh 검색 (Wraith Character 메인 Mesh의 child component — 소켓이 거기에 있음)
	UMeshComponent* BowMesh = nullptr;
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		TArray<UMeshComponent*> MeshComps;
		Avatar->GetComponents<UMeshComponent>(MeshComps);
		for (UMeshComponent* Comp : MeshComps)
		{
			if (Comp && Comp->GetName().Equals(TEXT("BowMesh"), ESearchCase::IgnoreCase))
			{
				BowMesh = Comp;
				break;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Wraith BowShove: BowMesh=%d Sockets Start=%d End=%d Radius=%.1f"),
		BowMesh ? 1 : 0,
		BowMesh ? (BowMesh->DoesSocketExist(StartSocketName) ? 1 : 0) : -1,
		BowMesh ? (BowMesh->DoesSocketExist(EndSocketName) ? 1 : 0) : -1,
		SweepRadius);

	// 휘두름 Cue
	if (UAbilitySystemComponent* SourceASC =
		GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParameters;
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			CueParameters.Location = Avatar->GetActorLocation();
			CueParameters.Normal = Avatar->GetActorForwardVector();
		}
		SourceASC->ExecuteGameplayCue(
			BlackoutGameplayTags::GameplayCue_Wraith_BowShove, CueParameters);
	}

	// 활대 스윕 시작 (BowMesh 소켓 사용)
	ActiveSweepTask = UAbilityTask_BossMeleeSweep::CreateSweepTask(
		this, StartSocketName, EndSocketName, SweepRadius, BowMesh);
	if (ActiveSweepTask)
	{
		ActiveSweepTask->OnHit.AddDynamic(
			this, &UGA_Wraith_BowShove::OnMeleeSweepHit);
		ActiveSweepTask->ReadyForActivation();
	}

	// 스윕 끝 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitEndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Enemy_Attack_SweepEnd, nullptr,
			true, true);
	
	if (WaitEndTask)
	{
		WaitEndTask->EventReceived.AddDynamic(this , &UGA_Wraith_BowShove::OnSweepEndEvent);
		WaitEndTask->ReadyForActivation();
	}
}

void UGA_Wraith_BowShove::OnSweepEndEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Wraith BowShove: SweepEnd"));

	if (ActiveSweepTask)
	{
		ActiveSweepTask->EndTask();
		ActiveSweepTask=nullptr;
	}
}

void UGA_Wraith_BowShove::OnMeleeSweepHit(const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Wraith BowShove: Hit %s @ %s BoneName=%s"),
		*GetNameSafe(HitResult.GetActor()),
		*HitResult.ImpactPoint.ToString(),
		*HitResult.BoneName.ToString());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
			FString::Printf(TEXT("BowShove Hit: %s"), *GetNameSafe(HitResult.GetActor())));
	}

	if (!DamageEffectClass)
	{
		return;
	}
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitResult.GetActor());
	if (!Damageable)
	{
		return;
	}
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
	{
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle SpecHandle= OwnerASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data -> SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage , DamageMagnitude);
		Damageable->ReceiveDamageFromHitbox(SpecHandle,HitResult.BoneName);
	}
}


void UGA_Wraith_BowShove::OnMontageEnded()
{
	if (ActiveSweepTask)
	{
		ActiveSweepTask->EndTask();
		ActiveSweepTask=nullptr;
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo,CurrentActivationInfo,true,false);
}
