#include "GAS/Abilities/Player/BlackoutGA_Reload.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
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
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

UBlackoutGA_Reload* UBlackoutGA_Reload::GetActiveReloadAbilityFromActor(const AActor* OwnerActor)
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
	const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			if (UBlackoutGA_Reload* ReloadAbility = Cast<UBlackoutGA_Reload>(AbilityInstance))
			{
				return ReloadAbility;
			}
		}
	}

	return nullptr;
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
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Reload::OnReloadMontageCompleted()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	BO_LOG_GAS(Log, "GA_Reload montage completed");

	// 장전 완료 시 데미지 이펙트 (ReloadEffectClass, 내부적으로 ExecCalc_Reload 실행) 적용하여 탄약 수치 갱신
	if (ReloadEffectClass && GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ReloadEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			if (PendingWeaponSlotTag.IsValid())
			{
				SpecHandle.Data->AddDynamicAssetTag(PendingWeaponSlotTag);
			}

			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Weapon_Reload);
			BO_LOG_GAS(Log, "GA_Reload applied reload effect");
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
