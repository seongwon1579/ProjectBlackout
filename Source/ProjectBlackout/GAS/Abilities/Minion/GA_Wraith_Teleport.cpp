// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_Wraith_Teleport.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"

UGA_Wraith_Teleport::UGA_Wraith_Teleport()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tag;
	Tag.AddTag(BlackoutGameplayTags::Ability_Wraith_Teleport);
	SetAssetTags(Tag);
}

void UGA_Wraith_Teleport::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo *ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData *TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo();
	AActor *Avatar = GetAvatarActorFromActorInfo();
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !ASC || !Avatar ||
		!TeleportQuery || !TeleportAnimMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 무적 (서버 -> 클라 동기화)
	ASC->AddLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);

	// 은신 Cue
	FGameplayCueParameters StartCueParams;
	StartCueParams.Location = Avatar->GetActorLocation();
	ASC->ExecuteGameplayCue(
		BlackoutGameplayTags::GameplayCue_Wraith_Teleport_Start,
		StartCueParams);

	// EQS 비동기 호출
	FEnvQueryRequest Request(TeleportQuery, Avatar);
	Request.Execute(EEnvQueryRunMode::SingleResult, this,
					&UGA_Wraith_Teleport::OnEQSFinished);
}

void UGA_Wraith_Teleport::OnEQSFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (!Result.IsValid() || !Result->IsSuccessful() || Result->Items.Num() == 0)
	{
		// 텔레포트 취소
		RemoveInvulnerableTag();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
				   true, false);
		return;
	}

	CachedDestination = Result->GetItemAsLocation(0);
	StartMontageAndWaitEvents();
}

void UGA_Wraith_Teleport::StartMontageAndWaitEvents()
{
	// Vanish 이벤트 대기
	UAbilityTask_WaitGameplayEvent *VanishTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Wraith_Teleport_Vanish,
			nullptr, true, true);

	if (VanishTask)
	{
		VanishTask->EventReceived.AddDynamic(this, &UGA_Wraith_Teleport::OnVanishEvent);
		VanishTask->ReadyForActivation();
	}

	// Appear 이벤트 대기
	UAbilityTask_WaitGameplayEvent *AppearTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, BlackoutGameplayTags::Event_Wraith_Teleport_Appear, nullptr, true, true);
	if (AppearTask)
	{
		AppearTask->EventReceived.AddDynamic(this, &UGA_Wraith_Teleport::OnAppearEvent);
		AppearTask->ReadyForActivation();
	}

	UAbilityTask_PlayMontageAndWait *MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TeleportAnimMontage);
	if (!MontageTask)
	{
		RemoveInvulnerableTag();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Wraith_Teleport::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Wraith_Teleport::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Wraith_Teleport::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Wraith_Teleport::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGA_Wraith_Teleport::OnVanishEvent(FGameplayEventData Payload)
{
	AActor *Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	SetTeleportVisualsHidden(true);
	Avatar->SetActorLocation(CachedDestination, false, nullptr, ETeleportType::TeleportPhysics);

	if (UAbilitySystemComponent *SourceASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters EndCueParams;
		EndCueParams.Location = CachedDestination;
		SourceASC->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Wraith_Teleport_End, EndCueParams);
	}
}

void UGA_Wraith_Teleport::OnAppearEvent(FGameplayEventData Payload)
{
	SetTeleportVisualsHidden(false);
}

void UGA_Wraith_Teleport::OnMontageEnded()
{
	SetTeleportVisualsHidden(false);
	RemoveInvulnerableTag();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Wraith_Teleport::RemoveInvulnerableTag()
{
	if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);
	}
}

void UGA_Wraith_Teleport::SetTeleportVisualsHidden(bool bHidden)
{
	AActor *Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	if (bHidden)
	{
		PreviousTeleportHiddenStates.Reset();
	}

	TArray<UPrimitiveComponent *> PrimitiveComponents;
	Avatar->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent *PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || PrimitiveComponent->IsA<UWidgetComponent>())
		{
			continue;
		}

		if (bHidden)
		{
			const TWeakObjectPtr<UPrimitiveComponent> ComponentKey(PrimitiveComponent);
			PreviousTeleportHiddenStates.Add(ComponentKey, PrimitiveComponent->bHiddenInGame);
			PrimitiveComponent->SetHiddenInGame(true, false);
			continue;
		}

		const TWeakObjectPtr<UPrimitiveComponent> ComponentKey(PrimitiveComponent);
		if (const bool *PreviousHiddenState = PreviousTeleportHiddenStates.Find(ComponentKey))
		{
			PrimitiveComponent->SetHiddenInGame(*PreviousHiddenState, false);
		}
	}

	if (!bHidden)
	{
		PreviousTeleportHiddenStates.Reset();
	}
}
