#include "Combat/Components/BlackoutCombatComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "AbilitySystemComponent.h"

UBlackoutCombatComponent::UBlackoutCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UBlackoutCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBlackoutCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UBlackoutCombatComponent, bIsAiming);
}

void UBlackoutCombatComponent::EquipPrimary()
{
	if (PrimaryWeapon && EquippedWeapon != PrimaryWeapon)
	{
		Server_EquipWeapon(PrimaryWeapon);
	}
}

void UBlackoutCombatComponent::EquipSecondary()
{
	if (SecondaryWeapon && EquippedWeapon != SecondaryWeapon)
	{
		Server_EquipWeapon(SecondaryWeapon);
	}
}

void UBlackoutCombatComponent::SwapWeapon()
{
	if (EquippedWeapon == PrimaryWeapon)
	{
		EquipSecondary();
	}
	else
	{
		EquipPrimary();
	}
}

void UBlackoutCombatComponent::StartFire()
{
	// ASC를 통해 GA_FireWeapon 트리거
}

void UBlackoutCombatComponent::StopFire()
{
	// GA_FireWeapon 중지
}

void UBlackoutCombatComponent::StartAim()
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent())
		{
			if (AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting))
			{
				return;
			}
		}
	}

	bIsAiming = true;
	// UI 및 카메라 업데이트
}

void UBlackoutCombatComponent::StopAim()
{
	bIsAiming = false;
	// UI 및 카메라 원상복구
}

void UBlackoutCombatComponent::TryReload()
{
	// ASC를 통해 GA_Reload 트리거
}

void UBlackoutCombatComponent::PerformMeleeHit()
{
	if (MeleeWeapon)
	{
		// MeleeWeapon->PerformSweep(...)
	}
}

FTransform UBlackoutCombatComponent::GetMuzzleTransform() const
{
	if (ABOFirearm* Firearm = Cast<ABOFirearm>(EquippedWeapon))
	{
		return Firearm->GetMuzzleTransform();
	}
	return FTransform::Identity;
}

FVector UBlackoutCombatComponent::GetAimImpactPoint() const
{
	// TODO: 카메라 전방 트레이스(Camera forward trace)
	return FVector::ZeroVector;
}

void UBlackoutCombatComponent::Server_EquipWeapon_Implementation(ABOWeaponBase* NewWeapon)
{
	EquippedWeapon = NewWeapon;
	OnRep_EquippedWeapon();
}

void UBlackoutCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon)
	{
		// 캐릭터의 손 소켓에 부착
		EquippedWeapon->AttachToOwner(TEXT("WeaponSocket"));
	}
}
