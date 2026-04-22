#include "Combat/Components/BlackoutCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Data/BOCharacterData.h"
#include "Engine/World.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Net/UnrealNetwork.h"

UBlackoutCombatComponent::UBlackoutCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UBlackoutCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBlackoutCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UBlackoutCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UBlackoutCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UBlackoutCombatComponent, MeleeWeapon);
	DOREPLIFETIME(UBlackoutCombatComponent, bIsAiming);
}

void UBlackoutCombatComponent::InitializeLoadoutFromCharacterData(const UBOCharacterData* CharacterData)
{
	if (!CharacterData || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!PrimaryWeapon && CharacterData->StartingPrimaryWeapon)
	{
		PrimaryWeapon = Cast<ABOFirearm>(SpawnWeaponActor(CharacterData->StartingPrimaryWeapon));
	}

	if (!SecondaryWeapon && CharacterData->StartingSecondaryWeapon)
	{
		SecondaryWeapon = Cast<ABOFirearm>(SpawnWeaponActor(CharacterData->StartingSecondaryWeapon));
	}

	if (!MeleeWeapon && CharacterData->StartingMeleeWeapon)
	{
		MeleeWeapon = Cast<ABOMeleeWeapon>(SpawnWeaponActor(CharacterData->StartingMeleeWeapon));
	}

	ApplyInitialAmmoLoadout();

	if (PrimaryWeapon)
	{
		Server_EquipWeapon_Implementation(PrimaryWeapon);
	}
	else if (SecondaryWeapon)
	{
		Server_EquipWeapon_Implementation(SecondaryWeapon);
	}
	else
	{
		RefreshWeaponAttachments();
	}
}

void UBlackoutCombatComponent::EquipPrimary()
{
	if (!PrimaryWeapon || EquippedWeapon == PrimaryWeapon)
	{
		return;
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_EquipWeapon_Implementation(PrimaryWeapon);
		return;
	}

	Server_EquipWeapon(PrimaryWeapon);
}

void UBlackoutCombatComponent::EquipSecondary()
{
	if (!SecondaryWeapon || EquippedWeapon == SecondaryWeapon)
	{
		return;
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_EquipWeapon_Implementation(SecondaryWeapon);
		return;
	}

	Server_EquipWeapon(SecondaryWeapon);
}

void UBlackoutCombatComponent::SwapWeapon()
{
	if (EquippedWeapon == PrimaryWeapon)
	{
		EquipSecondary();
		return;
	}

	EquipPrimary();
}

void UBlackoutCombatComponent::StartFire()
{
	HandleAbilityInputPressed(EBlackoutAbilityInputID::Fire);
}

void UBlackoutCombatComponent::StopFire()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Fire);
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

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_SetAiming(true);
	}
}

void UBlackoutCombatComponent::StopAim()
{
	bIsAiming = false;

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_SetAiming(false);
	}
}

void UBlackoutCombatComponent::TryReload()
{
	HandleAbilityInputPressed(EBlackoutAbilityInputID::Reload);
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Reload);
}

void UBlackoutCombatComponent::PerformMeleeHit()
{
	if (MeleeWeapon && GetOwner())
	{
		MeleeWeapon->PerformSweep(GetOwner()->GetActorForwardVector());
	}
}

ABOFirearm* UBlackoutCombatComponent::GetEquippedFirearm() const
{
	return Cast<ABOFirearm>(EquippedWeapon);
}

FGameplayTag UBlackoutCombatComponent::GetEquippedWeaponSlotTag() const
{
	if (EquippedWeapon && EquippedWeapon == SecondaryWeapon)
	{
		return BlackoutGameplayTags::Weapon_Secondary;
	}

	if (EquippedWeapon && EquippedWeapon == PrimaryWeapon)
	{
		return BlackoutGameplayTags::Weapon_Primary;
	}

	return FGameplayTag();
}

FTransform UBlackoutCombatComponent::GetMuzzleTransform() const
{
	if (const ABOFirearm* Firearm = Cast<ABOFirearm>(EquippedWeapon))
	{
		return Firearm->GetMuzzleTransform();
	}

	return FTransform::Identity;
}

FVector UBlackoutCombatComponent::GetAimImpactPoint() const
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AController* OwnerController = OwnerPawn->GetController())
		{
			FVector ViewLocation = FVector::ZeroVector;
			FRotator ViewRotation = FRotator::ZeroRotator;
			OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

			const FVector TraceStart = ViewLocation;
			const FVector TraceEnd = TraceStart + ViewRotation.Vector() * AimTraceDistance;

			FHitResult HitResult;
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutCombat_AimTrace), false, GetOwner());
			QueryParams.AddIgnoredActor(EquippedWeapon);

			if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
			{
				return HitResult.ImpactPoint;
			}

			return TraceEnd;
		}
	}

	if (GetOwner())
	{
		return GetMuzzleTransform().GetLocation() + GetOwner()->GetActorForwardVector() * AimTraceDistance;
	}

	return FVector::ZeroVector;
}

void UBlackoutCombatComponent::Server_EquipWeapon_Implementation(ABOWeaponBase* NewWeapon)
{
	EquippedWeapon = NewWeapon;
	OnRep_EquippedWeapon();
}

void UBlackoutCombatComponent::Server_SetAiming_Implementation(bool bNewAiming)
{
	bIsAiming = bNewAiming;
}

void UBlackoutCombatComponent::OnRep_EquippedWeapon()
{
	RefreshWeaponAttachments();
}

ABOWeaponBase* UBlackoutCombatComponent::SpawnWeaponActor(TSubclassOf<ABOWeaponBase> WeaponClass)
{
	if (!WeaponClass || !GetWorld() || !GetOwner())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABOWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<ABOWeaponBase>(WeaponClass, FTransform::Identity, SpawnParameters);
	if (SpawnedWeapon)
	{
		SpawnedWeapon->SetOwner(GetOwner());
	}

	return SpawnedWeapon;
}

void UBlackoutCombatComponent::RefreshWeaponAttachments() const
{
	auto AttachWeapon = [](ABOWeaponBase* Weapon, const FName SocketName)
	{
		if (!Weapon)
		{
			return;
		}

		Weapon->SetActorHiddenInGame(false);
		Weapon->AttachToOwner(SocketName);
	};

	AttachWeapon(PrimaryWeapon, EquippedWeapon == PrimaryWeapon ? EquippedWeaponSocketName : PrimaryHolsterSocketName);
	AttachWeapon(SecondaryWeapon, EquippedWeapon == SecondaryWeapon ? EquippedWeaponSocketName : SecondaryHolsterSocketName);
	AttachWeapon(MeleeWeapon, MeleeHolsterSocketName);
}

void UBlackoutCombatComponent::ApplyInitialAmmoLoadout() const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (!AbilitySystemInterface)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return;
	}

	const float PrimaryMagazineSize = PrimaryWeapon ? static_cast<float>(PrimaryWeapon->GetMagazineSize()) : 0.0f;
	const float PrimaryReserveAmmo = PrimaryWeapon ? static_cast<float>(PrimaryWeapon->GetMaxReserveAmmo()) : 0.0f;
	const float SecondaryMagazineSize = SecondaryWeapon ? static_cast<float>(SecondaryWeapon->GetMagazineSize()) : 0.0f;
	const float SecondaryReserveAmmo = SecondaryWeapon ? static_cast<float>(SecondaryWeapon->GetMaxReserveAmmo()) : 0.0f;

	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute(), PrimaryMagazineSize);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), PrimaryMagazineSize);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(), PrimaryReserveAmmo);

	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute(), SecondaryMagazineSize);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), SecondaryMagazineSize);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(), SecondaryReserveAmmo);
}

void UBlackoutCombatComponent::HandleAbilityInputPressed(EBlackoutAbilityInputID InputID) const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface
		? Cast<UBlackoutAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent())
		: nullptr)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(InputID));
	}
}

void UBlackoutCombatComponent::HandleAbilityInputReleased(EBlackoutAbilityInputID InputID) const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface
		? Cast<UBlackoutAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent())
		: nullptr)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(InputID));
	}
}
