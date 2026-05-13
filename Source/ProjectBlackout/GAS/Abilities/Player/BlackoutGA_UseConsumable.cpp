#include "GAS/Abilities/Player/BlackoutGA_UseConsumable.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "Framework/BlackoutPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "TimerManager.h"

UBlackoutGA_UseConsumable::UBlackoutGA_UseConsumable()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_UseConsumable);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_UseConsumable::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const UBOConsumableData* ResolvedConsumableData = ResolveConsumableData();
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	ABlackoutPlayerState* BlackoutPlayerState = ActorInfo ? Cast<ABlackoutPlayerState>(ActorInfo->OwnerActor.Get()) : nullptr;

	if (!ResolvedConsumableData || !ResolvedConsumableData->ConsumableTag.IsValid() || !AbilitySystemComponent || !BlackoutPlayerState)
	{
		BO_LOG_GAS(Warning, "소모품 사용 실패: Data, Tag, ASC 또는 PlayerState가 유효하지 않습니다. Data=%s ASC=%s PS=%s",
			*GetNameSafe(ResolvedConsumableData),
			*GetNameSafe(AbilitySystemComponent),
			*GetNameSafe(BlackoutPlayerState));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsConsumableCooldownReady(ResolvedConsumableData))
	{
		BO_LOG_GAS(Warning, "소모품 사용 실패: 아직 쿨다운 중입니다. Player=%s Tag=%s",
			*BlackoutPlayerState->GetPlayerName(),
			*ResolvedConsumableData->ConsumableTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (BlackoutPlayerState->GetConsumableCount(ResolvedConsumableData->ConsumableTag) < ConsumeAmount)
	{
		BO_LOG_GAS(Warning, "소모품 사용 실패: 수량이 부족합니다. Player=%s Tag=%s Current=%d Need=%d",
			*BlackoutPlayerState->GetPlayerName(),
			*ResolvedConsumableData->ConsumableTag.ToString(),
			BlackoutPlayerState->GetConsumableCount(ResolvedConsumableData->ConsumableTag),
			ConsumeAmount);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "소모품 사용 실패: CommitAbility 실패. Player=%s Tag=%s",
			*BlackoutPlayerState->GetPlayerName(),
			*ResolvedConsumableData->ConsumableTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bConsumableApplied = false;
	PendingConsumableData = ResolvedConsumableData;

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (ConsumableMontage)
	{
		if (PlayerCharacter)
		{
			const float MontageDuration = ConsumableMontage->GetPlayLength();
			if (MontageDuration > 0.f)
			{
				ApplySlowMovementSpeed(ActorInfo);
				BeginWeaponHolsterOverride(ActorInfo);
				if (PlayerCharacter->IsLocallyControlled())
				{
					PlayerCharacter->PlayAnimMontage(ConsumableMontage, 1.f);
				}

				if (PlayerCharacter->HasAuthority())
				{
					PlayerCharacter->Multicast_PlayConsumableMontage(ConsumableMontage, 1.f);

					UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackoutGameplayTags::Event_Montage_ConsumableApply);
					if (WaitEventTask)
					{
						WaitEventTask->EventReceived.AddDynamic(this, &UBlackoutGA_UseConsumable::OnConsumableApplyEventReceived);
						WaitEventTask->ReadyForActivation();
					}

					if (UWorld* World = PlayerCharacter->GetWorld())
					{
						World->GetTimerManager().SetTimer(ConsumableMontageTimerHandle, this, &UBlackoutGA_UseConsumable::OnConsumableMontageFinished, MontageDuration, false);
					}

					BO_LOG_GAS(Log, "소모품 몽타주 시작: Player=%s Data=%s Duration=%.2f",
						*BlackoutPlayerState->GetPlayerName(),
						*GetNameSafe(ResolvedConsumableData),
						MontageDuration);
				}

				return;
			}
		}
	}

	if (PlayerCharacter && PlayerCharacter->HasAuthority())
	{
		ConsumeAndApplyEffect();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBlackoutGA_UseConsumable::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ConsumableMontageTimerHandle);
		}

		if (bWasCancelled && ConsumableMontage)
		{
			if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
			{
				PlayerCharacter->Multicast_StopConsumableMontage(ConsumableMontage);
			}
		}

		RestoreMovementSpeed(ActorInfo);
		EndWeaponHolsterOverride(ActorInfo);
	}

	PendingConsumableData = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_UseConsumable::ConsumeAndApplyEffect()
{
	if (bConsumableApplied || !PendingConsumableData)
	{
		return;
	}

	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	bConsumableApplied = true;

	ABlackoutPlayerState* BlackoutPlayerState = CurrentActorInfo ? Cast<ABlackoutPlayerState>(CurrentActorInfo->OwnerActor.Get()) : nullptr;
	if (!BlackoutPlayerState || !BlackoutPlayerState->ConsumeConsumable(PendingConsumableData->ConsumableTag, ConsumeAmount))
	{
		BO_LOG_GAS(Warning, "소모품 차감 실패: Data=%s", *GetNameSafe(PendingConsumableData.Get()));
		return;
	}

	bKeepActiveAfterMontage = ApplyConsumableEffect(PendingConsumableData);
	ApplyConfiguredGameplayEffect(PendingConsumableData);
	StartConsumableCooldown(PendingConsumableData);

	ReceiveConsumableUsed(PendingConsumableData);
	BO_LOG_GAS(Log, "소모품 사용 완료: Player=%s Data=%s Tag=%s",
		*BlackoutPlayerState->GetPlayerName(),
		*GetNameSafe(PendingConsumableData.Get()),
		*PendingConsumableData->ConsumableTag.ToString());
}

void UBlackoutGA_UseConsumable::OnConsumableApplyEventReceived(FGameplayEventData Payload)
{
	ConsumeAndApplyEffect();
}

void UBlackoutGA_UseConsumable::OnConsumableMontageFinished()
{
	ConsumeAndApplyEffect();
	RestoreMovementSpeed(CurrentActorInfo);

	if (!bKeepActiveAfterMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

const UBOConsumableData* UBlackoutGA_UseConsumable::ResolveConsumableData()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent ? AbilitySystemComponent->FindAbilitySpecFromHandle(CurrentSpecHandle) : nullptr;
	if (AbilitySpec)
	{
		if (const UBOConsumableData* SourceConsumableData = Cast<UBOConsumableData>(AbilitySpec->SourceObject.Get()))
		{
			return SourceConsumableData;
		}
	}

	return ConsumableData;
}

bool UBlackoutGA_UseConsumable::ApplyConsumableEffect(const UBOConsumableData*)
{
	return false;
}

bool UBlackoutGA_UseConsumable::ShouldApplyConfiguredGameplayEffect(const UBOConsumableData*) const
{
	return true;
}

void UBlackoutGA_UseConsumable::ApplyConfiguredGameplayEffect(const UBOConsumableData* UsedConsumableData)
{
	if (!UsedConsumableData || !UsedConsumableData->GameplayEffect)
	{
		return;
	}

	if (!ShouldApplyConfiguredGameplayEffect(UsedConsumableData))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UsedConsumableData->GameplayEffect, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		BO_LOG_GAS(Warning, "소모품 GE 생성 실패: Data=%s Effect=%s",
			*GetNameSafe(UsedConsumableData),
			*GetNameSafe(UsedConsumableData->GameplayEffect.Get()));
		return;
	}

	SpecHandle.Data->AddDynamicAssetTag(UsedConsumableData->ConsumableTag);
	for (const TPair<FGameplayTag, float>& EffectMagnitude : UsedConsumableData->EffectMagnitudes)
	{
		if (EffectMagnitude.Key.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(EffectMagnitude.Key, EffectMagnitude.Value);
		}
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

float UBlackoutGA_UseConsumable::GetEffectMagnitudeOrDefault(const UBOConsumableData* UsedConsumableData, FGameplayTag MagnitudeTag, float DefaultValue) const
{
	if (!UsedConsumableData || !MagnitudeTag.IsValid())
	{
		return DefaultValue;
	}

	if (const float* FoundMagnitude = UsedConsumableData->EffectMagnitudes.Find(MagnitudeTag))
	{
		return *FoundMagnitude;
	}

	return DefaultValue;
}

bool UBlackoutGA_UseConsumable::IsConsumableCooldownReady(const UBOConsumableData* UsedConsumableData) const
{
	if (!UsedConsumableData || UsedConsumableData->Cooldown <= 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	return World->GetTimeSeconds() >= ConsumableCooldownEndTime;
}

void UBlackoutGA_UseConsumable::StartConsumableCooldown(const UBOConsumableData* UsedConsumableData)
{
	if (!UsedConsumableData || UsedConsumableData->Cooldown <= 0.0f)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		ConsumableCooldownEndTime = World->GetTimeSeconds() + UsedConsumableData->Cooldown;
	}
}

void UBlackoutGA_UseConsumable::ApplySlowMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	if (!MovementComponent)
	{
		return;
	}

	CachedWalkSpeed = MovementComponent->MaxWalkSpeed;
	MovementComponent->MaxWalkSpeed = CachedWalkSpeed * ConsumableSpeedMultiplier;

	if (PlayerCharacter->HasAuthority())
	{
		PlayerCharacter->Client_BeginAbilityMovementOverride(ConsumableSpeedMultiplier, false, false);
	}
}

void UBlackoutGA_UseConsumable::RestoreMovementSpeed(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (CachedWalkSpeed <= 0.0f)
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr)
	{
		MovementComponent->MaxWalkSpeed = CachedWalkSpeed;
	}

	if (PlayerCharacter && PlayerCharacter->HasAuthority())
	{
		PlayerCharacter->Client_EndAbilityMovementOverride();
	}

	CachedWalkSpeed = 0.0f;
}

void UBlackoutGA_UseConsumable::BeginWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UBlackoutCombatComponent>() : nullptr;
	if (!CombatComponent)
	{
		BO_LOG_GAS(Warning, "소모품 무기 홀스터 실패: CombatComponent가 유효하지 않습니다. Player=%s", *GetNameSafe(PlayerCharacter));
		return;
	}

	CombatComponent->BeginEquippedWeaponHolsterOverride();
}

void UBlackoutGA_UseConsumable::EndWeaponHolsterOverride(const FGameplayAbilityActorInfo* ActorInfo)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UBlackoutCombatComponent>() : nullptr;
	if (!CombatComponent)
	{
		return;
	}

	CombatComponent->EndEquippedWeaponHolsterOverride();
}
