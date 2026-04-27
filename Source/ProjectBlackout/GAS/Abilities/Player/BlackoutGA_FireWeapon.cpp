#include "GAS/Abilities/Player/BlackoutGA_FireWeapon.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Core/BlackoutLog.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GameFramework/Character.h"

UBlackoutGA_FireWeapon::UBlackoutGA_FireWeapon()
{
	InputID = EBlackoutAbilityInputID::Fire;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationRequiredTags.AddTag(BlackoutGameplayTags::State_Aiming);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	
	// TODO: LobbyTag.InfiniteAmmo 분기로 탄약 소모 체크 생략 로직 추가 (TDD §7.1)
}

void UBlackoutGA_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_FireWeapon activate requested");

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	if (!EquippedFirearm)
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon failed: 장착된 총기가 없음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 탄약 소모 확인 및 적용
	if (!ApplyAmmoCost())
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon failed: 탄약 부족");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_FireWeapon activated: Character=%s, Weapon=%s", *GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr), *GetNameSafe(EquippedFirearm));

	// 2. 사격 애니메이션 몽타주 재생
	PlayFireMontage();

	// 3. 트레이스 수행 또는 발사체 스폰 (무기 컴포넌트 정보 기반)
	const FVector MuzzleLocation = CombatComponent->GetMuzzleTransform().GetLocation();
	const FVector AimTarget = CombatComponent->GetAimImpactPoint();
	const FVector FireDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
	const FGameplayEffectSpecHandle DamageSpecHandle = BuildDamageSpec(EquippedFirearm);
	EquippedFirearm->Fire(FireDirection, DamageSpecHandle);

	// 4. 사격 GameplayCue 트리거
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Weapon_Fire);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UBlackoutGA_FireWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_FireWeapon ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FHitResult UBlackoutGA_FireWeapon::PerformTrace(const FVector& Start, const FVector& End)
{
	FHitResult HitResult;
	// TODO: 무기 유효 사거리 및 카메라-총구 간 크로스헤어 보정을 포함한 라인 트레이스 구현
	return HitResult;
}

FGameplayEffectSpecHandle UBlackoutGA_FireWeapon::BuildDamageSpec(const ABOFirearm* Firearm)
{
	FGameplayEffectSpecHandle SpecHandle;
	if (!DamageEffectClass)
	{
		BO_LOG_GAS(Error, "BuildDamageSpec failed: DamageEffectClass가 설정되지 않음 (Ability=%s)", *GetNameSafe(this));
		return SpecHandle;
	}

	if (Firearm && GetAbilitySystemComponentFromActorInfo())
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Firearm->GetBaseDamage());
		}
	}
	else
	{
		BO_LOG_GAS(Warning,
		           "BuildDamageSpec failed: Firearm=%s ASC=%s",
		           *GetNameSafe(Firearm),
		           *GetNameSafe(GetAbilitySystemComponentFromActorInfo()));
	}
	return SpecHandle;
}

bool UBlackoutGA_FireWeapon::ApplyAmmoCost()
{
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!CombatComponent || !AbilitySystemComponent)
	{
		return false;
	}

	const FGameplayTag WeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();
	if (WeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
	{
		const float SecondaryClipAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute());
		if (SecondaryClipAmmo < 1.0f)
		{
			return false;
		}

		AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
		return true;
	}

	const float PrimaryClipAmmo = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
	if (PrimaryClipAmmo < 1.0f)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
	return true;
}

void UBlackoutGA_FireWeapon::PlayFireMontage()
{
	// TODO: 캐릭터 사격 몽타주 재생 (PlayMontageAndWait 활용)
}
