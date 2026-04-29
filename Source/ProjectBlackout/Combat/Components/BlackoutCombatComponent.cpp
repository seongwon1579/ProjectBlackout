#include "Combat/Components/BlackoutCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Data/BOCharacterData.h"
#include "Engine/World.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"
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
	DOREPLIFETIME(UBlackoutCombatComponent, bMeleeWeaponAttachmentOverride);
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
	if (bIsWeaponSwapInProgress)
	{
		return;
	}

	StopAutomaticFire();
	ReleaseActivePrimaryAction();
	StopAim();

	const FGameplayTag TargetWeaponSlotTag = EquippedWeapon == PrimaryWeapon
		? BlackoutGameplayTags::Weapon_Secondary
		: BlackoutGameplayTags::Weapon_Primary;

	if (!BeginWeaponSwapInternal(TargetWeaponSlotTag))
	{
		if (TargetWeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
		{
			EquipSecondary();
			return;
		}

		EquipPrimary();
	}
}

void UBlackoutCombatComponent::StartFire()
{
	if (bIsWeaponSwapInProgress)
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Fire);
}

void UBlackoutCombatComponent::StopFire()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Fire);
}

void UBlackoutCombatComponent::HandlePrimaryActionPressed()
{
	if (bIsWeaponSwapInProgress)
	{
		return;
	}

	const EBlackoutAbilityInputID ResolvedInputID = ResolvePrimaryActionInputID();
	if (ResolvedInputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	ActivePrimaryActionInputID = ResolvedInputID;
	HandleAbilityInputPressed(ResolvedInputID);

	if (ResolvedInputID == EBlackoutAbilityInputID::Fire)
	{
		StartAutomaticFire();
		return;
	}

	if (ResolvedInputID == EBlackoutAbilityInputID::Melee || ResolvedInputID == EBlackoutAbilityInputID::Reload)
	{
		ReleaseActivePrimaryAction();
	}
}

void UBlackoutCombatComponent::HandlePrimaryActionReleased()
{
	StopAutomaticFire();
	ReleaseActivePrimaryAction();
}

void UBlackoutCombatComponent::StartAim()
{
	if (bIsWeaponSwapInProgress)
	{
		return;
	}

	if (!CanStartAim())
	{
		return;
	}

	ApplyAimingState(true);

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_SetAiming(true);
	}
}

void UBlackoutCombatComponent::StopAim()
{
	StopAutomaticFire();
	if (ActivePrimaryActionInputID == EBlackoutAbilityInputID::Fire)
	{
		ReleaseActivePrimaryAction();
	}

	ApplyAimingState(false);

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_SetAiming(false);
	}
}

void UBlackoutCombatComponent::TryReload()
{
	if (bIsWeaponSwapInProgress)
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Reload);
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Reload);
}

void UBlackoutCombatComponent::PerformMeleeHit(const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!MeleeWeapon)
	{
		return;
	}

	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	const TArray<FHitResult> HitResults = MeleeWeapon->PerformSweep(GetOwner()->GetActorForwardVector());
	TSet<TWeakObjectPtr<UPrimitiveComponent>> ProcessedComponents;
	TSet<TWeakObjectPtr<AActor>> ProcessedActors;

	for (const FHitResult& HitResult : HitResults)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		AActor* HitActor = HitResult.GetActor();
		if (!HitComponent && !HitActor)
		{
			continue;
		}

		if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent))
		{
			AActor* HitboxOwner = HitboxComponent->GetOwner();
			if (ProcessedComponents.Contains(HitboxComponent) || (HitboxOwner && ProcessedActors.Contains(HitboxOwner)))
			{
				continue;
			}

			ProcessedComponents.Add(HitboxComponent);
			if (HitboxOwner)
			{
				ProcessedActors.Add(HitboxOwner);
			}
			HitboxComponent->ReceiveDamageSpec(DamageSpecHandle);
			continue;
		}

		if (!HitActor || ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
		{
			ProcessedActors.Add(HitActor);
			Damageable->ReceiveDamageFromHitbox(DamageSpecHandle, HitResult.BoneName);
		}
	}
}

void UBlackoutCombatComponent::CommitPendingWeaponSwap()
{
	if (!PendingWeaponSwapSlotTag.IsValid())
	{
		return;
	}

	const FGameplayTag TargetWeaponSlotTag = PendingWeaponSwapSlotTag;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (ABOWeaponBase* TargetWeapon = ResolveWeaponBySlotTag(TargetWeaponSlotTag))
		{
			Server_EquipWeapon_Implementation(TargetWeapon);
		}

		PendingWeaponSwapSlotTag = FGameplayTag();
		return;
	}

	Server_CommitPendingWeaponSwap(TargetWeaponSlotTag);
	PendingWeaponSwapSlotTag = FGameplayTag();
}

void UBlackoutCombatComponent::BeginMeleeWeaponAttachmentOverride()
{
	if (!MeleeWeapon)
	{
		return;
	}

	bMeleeWeaponAttachmentOverride = true;
	RefreshWeaponAttachments();
}

void UBlackoutCombatComponent::EndMeleeWeaponAttachmentOverride()
{
	if (!bMeleeWeaponAttachmentOverride)
	{
		return;
	}

	bMeleeWeaponAttachmentOverride = false;
	RefreshWeaponAttachments();
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

			if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams))
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
	if (bNewAiming && !CanStartAim())
	{
		ApplyAimingState(false);
		return;
	}

	ApplyAimingState(bNewAiming);
}

void UBlackoutCombatComponent::Server_BeginWeaponSwap_Implementation(FGameplayTag TargetWeaponSlotTag)
{
	if (!ResolveWeaponBySlotTag(TargetWeaponSlotTag) || GetEquippedWeaponSlotTag() == TargetWeaponSlotTag)
	{
		return;
	}

	PendingWeaponSwapSlotTag = TargetWeaponSlotTag;
	bIsWeaponSwapInProgress = PendingWeaponSwapSlotTag.IsValid();

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetOwner()))
	{
		PlayerCharacter->Multicast_PlayWeaponSwapMontage(TargetWeaponSlotTag, 1.f);
	}
}

void UBlackoutCombatComponent::Server_CommitPendingWeaponSwap_Implementation(FGameplayTag TargetWeaponSlotTag)
{
	if (!PendingWeaponSwapSlotTag.IsValid() || PendingWeaponSwapSlotTag != TargetWeaponSlotTag)
	{
		return;
	}

	if (ABOWeaponBase* TargetWeapon = ResolveWeaponBySlotTag(TargetWeaponSlotTag))
	{
		Server_EquipWeapon_Implementation(TargetWeapon);
	}

	PendingWeaponSwapSlotTag = FGameplayTag();
}

void UBlackoutCombatComponent::Server_CancelPendingWeaponSwap_Implementation()
{
	PendingWeaponSwapSlotTag = FGameplayTag();
	bIsWeaponSwapInProgress = false;
}

void UBlackoutCombatComponent::OnRep_EquippedWeapon()
{
	RefreshWeaponAttachments();
}

void UBlackoutCombatComponent::OnRep_LoadoutWeapon()
{
	RefreshWeaponAttachments();
}

void UBlackoutCombatComponent::OnRep_IsAiming()
{
	ApplyAimingState(bIsAiming);
}

void UBlackoutCombatComponent::OnRep_MeleeWeaponAttachmentOverride()
{
	RefreshWeaponAttachments();
}

void UBlackoutCombatComponent::HandleWeaponSwapMontageEnded(bool bInterrupted)
{
	if (!bInterrupted && PendingWeaponSwapSlotTag.IsValid())
	{
		CommitPendingWeaponSwap();
	}

	if (bInterrupted && PendingWeaponSwapSlotTag.IsValid() && GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_CancelPendingWeaponSwap();
	}

	if (bInterrupted)
	{
		PendingWeaponSwapSlotTag = FGameplayTag();
	}

	bIsWeaponSwapInProgress = false;
}

void UBlackoutCombatComponent::BeginMeleeAttackWindow(const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	// 혹시 이전 공격창 상태가 남아 있으면 먼저 정리
	EndMeleeAttackWindow();
	
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!MeleeWeapon)
	{
		return;
	}

	
	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	// 이번 공격창에서 재사용할 데미지 스펙 저장
	ActiveMeleeDamageSpecHandle = DamageSpecHandle;

	// 이번 공격창 중복 피격 방지 집합 초기화
	ProcessedMeleeHitComponents.Reset();
	ProcessedMeleeHitActors.Reset();

	// 공격창 활성화
	bMeleeAttackWindowActive = true;
	
}

void UBlackoutCombatComponent::UpdateMeleeAttackWindow()
{
	if (!bMeleeAttackWindowActive)
	{
		return;
	}

	// 서버만 실제 데미지 판정을 수행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// 공격창이 열려 있어도 무기나 스펙이 없으면 진행 불가
	if (!MeleeWeapon || !ActiveMeleeDamageSpecHandle.IsValid())
	{
		return;
	}

	// 현재 프레임 기준으로 근접 무기 스윕 수행
	const TArray<FHitResult> HitResults = MeleeWeapon->PerformSweep(GetOwner()->GetActorForwardVector());

	for (const FHitResult& HitResult : HitResults)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		AActor* HitActor = HitResult.GetActor();

		// 히트박스 컴포넌트가 잡힌 경우
		if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent))
		{
			AActor* HitboxOwner = HitboxComponent->GetOwner();

			// 같은 공격창 안에서 이미 처리한 히트박스/액터면 중복 피해 방지
			if (ProcessedMeleeHitComponents.Contains(HitboxComponent)
				|| (HitboxOwner && ProcessedMeleeHitActors.Contains(HitboxOwner)))
			{
				continue;
			}

			// 이번 공격창에서 이미 맞은 대상 목록에 기록
			ProcessedMeleeHitComponents.Add(HitboxComponent);
			if (HitboxOwner)
			{
				ProcessedMeleeHitActors.Add(HitboxOwner);
			}

			// 히트박스 컴포넌트 경유 데미지 전달
			HitboxComponent->ReceiveDamageSpec(ActiveMeleeDamageSpecHandle);
			continue;
		}

		// 히트박스가 아닌 일반 액터인 경우
		if (!HitActor || ProcessedMeleeHitActors.Contains(HitActor))
		{
			continue;
		}

		if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
		{
			// 같은 공격창 안에서 동일 액터 중복 피해 방지
			ProcessedMeleeHitActors.Add(HitActor);

			// 본 정보가 있으면 함께 전달
			Damageable->ReceiveDamageFromHitbox(ActiveMeleeDamageSpecHandle, HitResult.BoneName);
		}
	}
	
}

void UBlackoutCombatComponent::EndMeleeAttackWindow()
{
	// 공격창 비활성화
	bMeleeAttackWindowActive = false;

	// 이번 공격창용 임시 상태 정리
	ActiveMeleeDamageSpecHandle = FGameplayEffectSpecHandle();
	ProcessedMeleeHitComponents.Reset();
	ProcessedMeleeHitActors.Reset();
	
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
		SpawnedWeapon->SetActorHiddenInGame(true);
		SpawnedWeapon->InitializeStatsFromDataTable();
	}

	return SpawnedWeapon;
}

void UBlackoutCombatComponent::RefreshWeaponAttachments() const
{
	auto AttachWeapon = [this](ABOWeaponBase* Weapon, const bool bAttachAsEquipped)
	{
		if (!Weapon)
		{
			return;
		}

		Weapon->InitializeStatsFromDataTable();

		const FName SocketName = bAttachAsEquipped ? EquippedWeaponSocketName : Weapon->GetHolsterSocketName();
		if (SocketName.IsNone())
		{
			return;
		}

		if (Weapon->AttachToOwner(SocketName))
		{
			Weapon->SetActorHiddenInGame(false);
		}
	};

	const bool bPrimaryEquipped = EquippedWeapon == PrimaryWeapon;
	const bool bSecondaryEquipped = EquippedWeapon == SecondaryWeapon;
	const bool bMeleeEquipped = bMeleeWeaponAttachmentOverride && MeleeWeapon;

	AttachWeapon(PrimaryWeapon, !bMeleeEquipped && bPrimaryEquipped);
	AttachWeapon(SecondaryWeapon, !bMeleeEquipped && bSecondaryEquipped);
	AttachWeapon(MeleeWeapon, bMeleeEquipped);
}

ABOWeaponBase* UBlackoutCombatComponent::ResolveWeaponBySlotTag(FGameplayTag WeaponSlotTag) const
{
	if (WeaponSlotTag == BlackoutGameplayTags::Weapon_Primary)
	{
		return PrimaryWeapon;
	}

	if (WeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
	{
		return SecondaryWeapon;
	}

	return nullptr;
}

bool UBlackoutCombatComponent::BeginWeaponSwapInternal(FGameplayTag TargetWeaponSlotTag)
{
	ABOWeaponBase* TargetWeapon = ResolveWeaponBySlotTag(TargetWeaponSlotTag);
	if (!TargetWeapon || TargetWeapon == EquippedWeapon)
	{
		return false;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetOwner());
	if (!PlayerCharacter || !PlayerCharacter->PlayWeaponSwapMontage(TargetWeaponSlotTag, 1.f))
	{
		return false;
	}

	PendingWeaponSwapSlotTag = TargetWeaponSlotTag;
	bIsWeaponSwapInProgress = true;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PlayerCharacter->Multicast_PlayWeaponSwapMontage(TargetWeaponSlotTag, 1.f);
	}
	else
	{
		Server_BeginWeaponSwap(TargetWeaponSlotTag);
	}

	return true;
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

bool UBlackoutCombatComponent::CanStartAim() const
{
	if (!GetEquippedFirearm())
	{
		return false;
	}

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		return false;
	}

	return !AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Attacking);
}

float UBlackoutCombatComponent::GetEquippedClipAmmo() const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		return 0.0f;
	}

	if (GetEquippedWeaponSlotTag() == BlackoutGameplayTags::Weapon_Secondary)
	{
		return AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute());
	}

	return AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
}

void UBlackoutCombatComponent::ApplyAimingState(bool bNewAiming)
{
	bIsAiming = bNewAiming;

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(BlackoutGameplayTags::State_Aiming, bIsAiming ? 1 : 0);
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetOwner()))
	{
		PlayerCharacter->HandleAimStateChanged(bIsAiming);
	}
}

EBlackoutAbilityInputID UBlackoutCombatComponent::ResolvePrimaryActionInputID() const
{
	if (bIsAiming)
	{
		if (!GetEquippedFirearm())
		{
			return EBlackoutAbilityInputID::None;
		}

		return GetEquippedClipAmmo() > 0.0f ? EBlackoutAbilityInputID::Fire : EBlackoutAbilityInputID::Reload;
	}

	return MeleeWeapon ? EBlackoutAbilityInputID::Melee : EBlackoutAbilityInputID::None;
}

void UBlackoutCombatComponent::StartAutomaticFire()
{
	const ABOFirearm* EquippedFirearm = GetEquippedFirearm();
	if (!EquippedFirearm || !EquippedFirearm->IsAutomatic() || !GetWorld())
	{
		return;
	}

	const float FireInterval = 1.0f / FMath::Max(EquippedFirearm->GetFireRate(), 0.01f);
	GetWorld()->GetTimerManager().SetTimer(
		AutomaticFireTimerHandle,
		this,
		&UBlackoutCombatComponent::HandleAutomaticFireTick,
		FireInterval,
		true);
}

void UBlackoutCombatComponent::StopAutomaticFire()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticFireTimerHandle);
	}
}

void UBlackoutCombatComponent::HandleAutomaticFireTick()
{
	if (!CanStartAim())
	{
		StopAutomaticFire();
		ReleaseActivePrimaryAction();
		return;
	}

	if (ActivePrimaryActionInputID != EBlackoutAbilityInputID::Fire || ResolvePrimaryActionInputID() != EBlackoutAbilityInputID::Fire)
	{
		StopAutomaticFire();
		ReleaseActivePrimaryAction();

		if (bIsAiming && GetEquippedFirearm())
		{
			HandleAbilityInputPressed(EBlackoutAbilityInputID::Reload);
			HandleAbilityInputReleased(EBlackoutAbilityInputID::Reload);
		}
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Fire);
}

void UBlackoutCombatComponent::ReleaseActivePrimaryAction()
{
	if (ActivePrimaryActionInputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	HandleAbilityInputReleased(ActivePrimaryActionInputID);
	ActivePrimaryActionInputID = EBlackoutAbilityInputID::None;
}

void UBlackoutCombatComponent::HandleAbilityInputPressed(EBlackoutAbilityInputID InputID) const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface
		? Cast<UBlackoutAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent())
		: nullptr)
	{
		AbilitySystemComponent->HandleAbilityInputPressed(InputID);
	}
}

void UBlackoutCombatComponent::HandleAbilityInputReleased(EBlackoutAbilityInputID InputID) const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface
		? Cast<UBlackoutAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent())
		: nullptr)
	{
		AbilitySystemComponent->HandleAbilityInputReleased(InputID);
	}
}
