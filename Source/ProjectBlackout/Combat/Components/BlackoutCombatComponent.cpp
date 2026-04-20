#include "Combat/Components/BlackoutCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOMeleeWeapon.h"

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
	// Trigger GA_FireWeapon via ASC
}

void UBlackoutCombatComponent::StopFire()
{
	// Stop GA_FireWeapon
}

void UBlackoutCombatComponent::StartAim()
{
	bIsAiming = true;
	// Update UI/Camera
}

void UBlackoutCombatComponent::StopAim()
{
	bIsAiming = false;
	// Revert UI/Camera
}

void UBlackoutCombatComponent::TryReload()
{
	// Trigger GA_Reload via ASC
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
	// TODO: Camera forward trace
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
		// Attach to character's hand socket
		EquippedWeapon->AttachToOwner(TEXT("WeaponSocket"));
	}
}
