#include "GAS/Abilities/Player/BlackoutGA_Reload.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Core/BlackoutLog.h"
#include "Engine/World.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Reload::UBlackoutGA_Reload()
{
	InputID = EBlackoutAbilityInputID::Reload;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Reloading);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Attacking);
}

UAnimMontage* UBlackoutGA_Reload::ResolveReloadMontage(const ABlackoutPlayerCharacter* PlayerCharacter, const ABOFirearm* EquippedFirearm) const
{
	if (!PlayerCharacter || !EquippedFirearm)
	{
		return nullptr;
	}

	return PlayerCharacter->GetReloadMontageForTag(
		EquippedFirearm->GetReloadAnimTag(),
		EquippedFirearm->UsesTwoHandedAnimation());
}

void UBlackoutGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Reload activate requested");

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerCharacter || !CombatComponent || !EquippedFirearm || !AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_Reload failed: PlayerCharacter, CombatComponent, 무기 또는 ASC가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PendingWeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();

	float CurrentClipAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
	float CurrentMaxClip = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute());
	float CurrentReserveAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute());

	if (PendingWeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
	{
		CurrentClipAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute());
		CurrentMaxClip = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute());
		CurrentReserveAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute());
	}

	if (CurrentClipAmmo >= CurrentMaxClip || CurrentReserveAmmo <= 0.0f)
	{
		BO_LOG_GAS(Warning, "GA_Reload failed: 장전 불필요 또는 예비 탄약 없음 (Clip=%.2f, MaxClip=%.2f, Reserve=%.2f)", CurrentClipAmmo, CurrentMaxClip, CurrentReserveAmmo);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Reload failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* SelectedReloadMontage = ResolveReloadMontage(PlayerCharacter, EquippedFirearm);
	CachedReloadMontage = SelectedReloadMontage;
	bWeaponReloadAnimationTriggered = false;
	bReloadEffectApplied = false;
	const bool bIsTwoHanded = EquippedFirearm->UsesTwoHandedAnimation();

	BO_LOG_GAS(Log,
		"GA_Reload activated: Character=%s, Weapon=%s, TwoHanded=%s, Montage=%s",
		*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr),
		*GetNameSafe(EquippedFirearm),
		bIsTwoHanded ? TEXT("true") : TEXT("false"),
		*GetNameSafe(SelectedReloadMontage));

	if (SelectedReloadMontage)
	{
		if (UAbilityTask_WaitGameplayEvent* WaitEventTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackoutGameplayTags::Event_Montage_ReloadWeaponStart))
		{
			WaitEventTask->EventReceived.AddDynamic(this, &UBlackoutGA_Reload::OnWeaponReloadStartEventReceived);
			WaitEventTask->ReadyForActivation();
		}

		if (UAbilityTask_WaitGameplayEvent* WaitAmmoCommitTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackoutGameplayTags::Event_Montage_ReloadAmmoCommit))
		{
			WaitAmmoCommitTask->EventReceived.AddDynamic(this, &UBlackoutGA_Reload::OnReloadAmmoCommitEventReceived);
			WaitAmmoCommitTask->ReadyForActivation();
		}

		if (PlayerCharacter->IsLocallyControlled())
		{
			PlayerCharacter->PlayReloadMontage(SelectedReloadMontage, 1.f);
		}

		if (PlayerCharacter->HasAuthority())
		{
			PlayerCharacter->Multicast_PlayReloadMontage(SelectedReloadMontage, 1.f);

			const float MontageDuration = SelectedReloadMontage->GetPlayLength();
			if (MontageDuration > 0.0f)
			{
				if (UWorld* World = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr)
				{
					World->GetTimerManager().SetTimer(ReloadCompletionTimerHandle, this, &UBlackoutGA_Reload::OnReloadMontageCompleted, MontageDuration, false);
				}
				return;
			}
		}
	}

	if (PlayerCharacter->HasAuthority())
	{
		ApplyReloadEffect();
		OnReloadMontageCompleted();
	}
}

void UBlackoutGA_Reload::OnWeaponReloadStartEventReceived(FGameplayEventData Payload)
{
	if (bWeaponReloadAnimationTriggered)
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	if (!PlayerCharacter || !EquippedFirearm)
	{
		BO_LOG_GAS(Warning, "GA_Reload weapon reload event ignored: PlayerCharacter 또는 EquippedFirearm이 유효하지 않음");
		return;
	}

	bWeaponReloadAnimationTriggered = true;

	if (PlayerCharacter->IsLocallyControlled())
	{
		EquippedFirearm->PlayWeaponReloadAnimation();
	}

	if (PlayerCharacter->HasAuthority())
	{
		EquippedFirearm->Multicast_PlayWeaponReloadAnimation();
	}

	BO_LOG_GAS(Log,
		"GA_Reload weapon reload event received: Character=%s Weapon=%s EventTag=%s",
		*GetNameSafe(PlayerCharacter),
		*GetNameSafe(EquippedFirearm),
		*Payload.EventTag.ToString());
}

void UBlackoutGA_Reload::OnReloadAmmoCommitEventReceived(FGameplayEventData Payload)
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_Reload ammo commit ignored: ActorInfo 또는 AvatarActor가 유효하지 않음");
		return;
	}

	if (!CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	BO_LOG_GAS(Log,
		"GA_Reload ammo commit event received: Character=%s EventTag=%s",
		*GetNameSafe(CurrentActorInfo->AvatarActor.Get()),
		*Payload.EventTag.ToString());

	ApplyReloadEffect();
}

void UBlackoutGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Reload ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
		UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
		ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;

		if (bWasCancelled && PlayerCharacter && CachedReloadMontage)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				PlayerCharacter->StopReloadMontage(CachedReloadMontage, 0.1f);
			}

			if (PlayerCharacter->HasAuthority())
			{
				PlayerCharacter->Multicast_StopReloadMontage(CachedReloadMontage, 0.1f);
			}
		}

		if (bWasCancelled && bWeaponReloadAnimationTriggered && EquippedFirearm)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				EquippedFirearm->StopWeaponReloadAnimation();
			}

			if (PlayerCharacter->HasAuthority())
			{
				EquippedFirearm->Multicast_StopWeaponReloadAnimation();
			}
		}

		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ReloadCompletionTimerHandle);
		}
	}

	PendingWeaponSlotTag = FGameplayTag();
	CachedReloadMontage = nullptr;
	bWeaponReloadAnimationTriggered = false;
	bReloadEffectApplied = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Reload::OnReloadMontageCompleted()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	BO_LOG_GAS(Log, "GA_Reload montage completed");

	if (!bReloadEffectApplied)
	{
		BO_LOG_GAS(Warning, "GA_Reload completed without ammo commit notify: Montage=%s", *GetNameSafe(CachedReloadMontage));
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBlackoutGA_Reload::ApplyReloadEffect()
{
	if (bReloadEffectApplied)
	{
		return;
	}

	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_Reload apply effect failed: ASC가 유효하지 않음");
		return;
	}

	if (ReloadEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ReloadEffectClass, GetAbilityLevel());
		if (!SpecHandle.IsValid())
		{
			BO_LOG_GAS(Warning, "GA_Reload apply effect failed: GameplayEffectSpec 생성 실패");
			return;
		}

		if (PendingWeaponSlotTag.IsValid())
		{
			SpecHandle.Data->AddDynamicAssetTag(PendingWeaponSlotTag);
		}

		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		AbilitySystemComponent->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Weapon_Reload);
		bReloadEffectApplied = true;
		BO_LOG_GAS(Log, "GA_Reload applied reload effect");
		return;
	}

	BO_LOG_GAS(Warning, "GA_Reload apply effect failed: ReloadEffectClass가 설정되지 않음");
}
