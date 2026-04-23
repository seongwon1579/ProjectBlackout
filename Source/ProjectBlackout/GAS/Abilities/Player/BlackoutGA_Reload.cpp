#include "GAS/Abilities/Player/BlackoutGA_Reload.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Core/BlackoutLog.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Reload::UBlackoutGA_Reload()
{
	InputID = EBlackoutAbilityInputID::Reload;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UBlackoutGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Reload activate requested");

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!CombatComponent || !EquippedFirearm || !AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_Reload failed: CombatComponent, 무기 또는 ASC가 유효하지 않음");
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

	BO_LOG_GAS(Log, "GA_Reload activated: Character=%s, Weapon=%s", *GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr), *GetNameSafe(EquippedFirearm));

	// 1. 장전 애니메이션 몽타주 재생
	if (ReloadMontage)
	{
		float MontageDuration = 0.0f;

		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
			{
				MontageDuration = AnimInstance->Montage_Play(ReloadMontage);
			}
		}

		if (MontageDuration > 0.0f)
		{
			if (UWorld* World = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr)
			{
				World->GetTimerManager().SetTimer(ReloadCompletionTimerHandle, this, &UBlackoutGA_Reload::OnReloadMontageCompleted, MontageDuration, false);
				return;
			}
		}
	}

	OnReloadMontageCompleted();
}

void UBlackoutGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Reload ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ReloadCompletionTimerHandle);
		}
	}

	PendingWeaponSlotTag = FGameplayTag();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Reload::OnReloadMontageCompleted()
{
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
