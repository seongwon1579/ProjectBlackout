// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_TeleportBase.h"

#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UBlackoutGA_Shrewd_TeleportBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	SetTeleportVisualsHidden(false);
	RemoveInvulnerableTag();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(BlackoutGameplayTags::State_MovementLocked);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UBlackoutGA_Shrewd_TeleportBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	// 쿨다운 중이면 발동 불가 → ActivateAbility Task가 Failed → At Random 재추첨(연속 텔포 차단)
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() &&
		ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::Cooldown_Shrewd_Teleport))
	{
		return false;
	}
	return true;
}

void UBlackoutGA_Shrewd_TeleportBase::PrepareAbility()
{
	// 텔레포트 능력 활성 동안 이동 잠금 (FlyKite가 목적지를 끌어당기지 않게)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(BlackoutGameplayTags::State_MovementLocked);
	}
	StartResolveDestination();
}

void UBlackoutGA_Shrewd_TeleportBase::SetupEventListeners()
{
	// Vanish 이벤트 대기
	UAbilityTask_WaitGameplayEvent* VanishTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Wraith_Teleport_Vanish, nullptr, true, true);
	if (VanishTask)
	{
		VanishTask->EventReceived.AddDynamic(this, &UBlackoutGA_Shrewd_TeleportBase::OnVanishEvent);
		VanishTask->ReadyForActivation();
	}

	// Appear 이벤트 대기
	UAbilityTask_WaitGameplayEvent* AppearTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, BlackoutGameplayTags::Event_Wraith_Teleport_Appear, nullptr, true, true);
	if (AppearTask)
	{
		AppearTask->EventReceived.AddDynamic(this, &UBlackoutGA_Shrewd_TeleportBase::OnAppearEvent);
		AppearTask->ReadyForActivation();
	}
}

void UBlackoutGA_Shrewd_TeleportBase::OnVanishEvent(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);
	}

	SetTeleportVisualsHidden(true);

	FVector TeleportDest = CachedDestination;
	if (const ACharacter* AvatarChar = Cast<ACharacter>(Avatar))
	{
		const float HalfHeight = AvatarChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		TeleportDest.Z += HalfHeight;
	}

	const FRotator TeleportRot(0.f, CachedTeleportRotation.Yaw, 0.f);
	Avatar->SetActorLocationAndRotation(TeleportDest, TeleportRot, false, nullptr, ETeleportType::TeleportPhysics);

	if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters EndCueParams;
		EndCueParams.Location = TeleportDest;
		SourceASC->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Wraith_Teleport_End, EndCueParams);
	}

	// 텔레포트 쿨다운 — 연속 텔포 방지 (TeleportCooldown 초간 재발동 차단, 타이머로 자동 해제)
	if (UAbilitySystemComponent* CooldownASC = GetAbilitySystemComponentFromActorInfo())
	{
		CooldownASC->AddLooseGameplayTag(BlackoutGameplayTags::Cooldown_Shrewd_Teleport);
		if (UWorld* World = GetWorld())
		{
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC = CooldownASC;
			FTimerHandle CooldownTimer;
			World->GetTimerManager().SetTimer(CooldownTimer, [WeakASC]()
			{
				if (WeakASC.IsValid())
				{
					WeakASC->RemoveLooseGameplayTag(BlackoutGameplayTags::Cooldown_Shrewd_Teleport);
				}
			}, TeleportCooldown, false);
		}
	}
}

void UBlackoutGA_Shrewd_TeleportBase::OnAppearEvent(FGameplayEventData Payload)
{
	SetTeleportVisualsHidden(false);
	RemoveInvulnerableTag();
}

void UBlackoutGA_Shrewd_TeleportBase::RemoveInvulnerableTag()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Invulnerable))
		{
			ASC->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);
		}
	}
}

void UBlackoutGA_Shrewd_TeleportBase::SetTeleportVisualsHidden(bool bHidden)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	if (bHidden)
	{
		PreviousTeleportHiddenStates.Reset();
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Avatar->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
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
		if (const bool* PreviousHiddenState = PreviousTeleportHiddenStates.Find(ComponentKey))
		{
			PrimitiveComponent->SetHiddenInGame(*PreviousHiddenState, false);
		}
	}

	if (!bHidden)
	{
		PreviousTeleportHiddenStates.Reset();
	}
}
