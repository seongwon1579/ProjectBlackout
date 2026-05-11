#include "GAS/Abilities/Player/BlackoutGA_SwapWeapon.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutGA_SwapWeapon::UBlackoutGA_SwapWeapon()
{
	InputID = EBlackoutAbilityInputID::SwapWeapon;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(BlackoutGameplayTags::Ability_Player_SwapWeapon);
	SetAssetTags(AssetTags);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Attacking);
}

void UBlackoutGA_SwapWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_SwapWeapon activate requested");

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	if (!PlayerCharacter || !CombatComponent)
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon failed: PlayerCharacter 또는 CombatComponent가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedTargetWeaponSlotTag = CombatComponent->GetEquippedWeapon() == CombatComponent->GetPrimaryWeapon()
		? BlackoutGameplayTags::Weapon_Secondary
		: BlackoutGameplayTags::Weapon_Primary;
	CachedWeaponSwapMontage = PlayerCharacter->GetWeaponSwapMontageForSlot(CachedTargetWeaponSlotTag);
	bWeaponSwapCommitted = false;

	if (CachedWeaponSwapMontage)
	{
		if (UAbilityTask_WaitGameplayEvent* WaitCommitTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackoutGameplayTags::Event_Montage_WeaponSwapCommit))
		{
			WaitCommitTask->EventReceived.AddDynamic(this, &UBlackoutGA_SwapWeapon::OnWeaponSwapCommitEventReceived);
			WaitCommitTask->ReadyForActivation();
		}
	}

	CombatComponent->SwapWeapon();

	if (!CombatComponent->IsWeaponSwapInProgress())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (PlayerCharacter->HasAuthority() && CachedWeaponSwapMontage)
	{
		const float MontageDuration = CachedWeaponSwapMontage->GetPlayLength();
		if (MontageDuration > 0.0f)
		{
			if (UAbilityTask_WaitDelay* WaitMontageEndTask = UAbilityTask_WaitDelay::WaitDelay(this, MontageDuration))
			{
				WaitMontageEndTask->OnFinish.AddDynamic(this, &UBlackoutGA_SwapWeapon::OnWeaponSwapMontageCompleted);
				WaitMontageEndTask->ReadyForActivation();
			}
			return;
		}

		BO_LOG_GAS(Warning, "GA_SwapWeapon warning: 무기 교체 몽타주 길이가 0 이하임 Montage=%s", *GetNameSafe(CachedWeaponSwapMontage));
	}

	if (PlayerCharacter->HasAuthority())
	{
		OnWeaponSwapMontageCompleted();
	}
}

void UBlackoutGA_SwapWeapon::OnWeaponSwapCommitEventReceived(FGameplayEventData Payload)
{
	if (bWeaponSwapCommitted)
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	if (!PlayerCharacter || !CombatComponent)
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon commit event ignored: PlayerCharacter 또는 CombatComponent가 유효하지 않음");
		return;
	}

	bWeaponSwapCommitted = true;
	CombatComponent->CommitPendingWeaponSwap();

	BO_LOG_GAS(Log,
		"GA_SwapWeapon commit event received: Character=%s EventTag=%s",
		*GetNameSafe(PlayerCharacter),
		*Payload.EventTag.ToString());
}

void UBlackoutGA_SwapWeapon::OnWeaponSwapMontageCompleted()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const bool bWeaponAlreadySwapped = CombatComponent && CombatComponent->GetEquippedWeaponSlotTag() == CachedTargetWeaponSlotTag;
	if (CombatComponent)
	{
		CombatComponent->HandleWeaponSwapMontageEnded(false);
	}

	if (!bWeaponSwapCommitted && !bWeaponAlreadySwapped)
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon completed without weapon swap commit notify: Montage=%s", *GetNameSafe(CachedWeaponSwapMontage));
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBlackoutGA_SwapWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_SwapWeapon ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (bWasCancelled)
		{
			ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
			UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
			if (CombatComponent)
			{
				CombatComponent->HandleWeaponSwapMontageEnded(true);
			}
		}
	}

	CachedWeaponSwapMontage = nullptr;
	CachedTargetWeaponSlotTag = FGameplayTag();
	bWeaponSwapCommitted = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
