#include "UI/BlackoutHUDWidgetController.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Core/BlackoutLog.h"
#include "Data/BOCharacterData.h"
#include "Data/BOConsumableData.h"
#include "Framework/BlackoutPlayerController.h"
#include "Framework/BlackoutPlayerState.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Blueprint/WidgetLayoutLibrary.h"

bool UBlackoutHUDWidgetController::Initialize(APlayerController* InPlayerController)
{
	if (!ResolveDependencies(InPlayerController))
	{
		return false;
	}

	BindCallbacksToDependencies();
	return true;
}

void UBlackoutHUDWidgetController::BindCallbacksToDependencies()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC && !bAttributeCallbacksBound)
	{
		BO_LOG_CORE(Error, "HUD 바인딩 실패: AbilitySystemComponent가 유효하지 않습니다.");
		return;
	}

	if (ASC && !bAttributeCallbacksBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleMaxHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutPlayerAttributeSet::GetStaminaAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleStaminaChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleMaxStaminaChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleAmmoChanged);

		bAttributeCallbacksBound = true;
	}

	if (ABlackoutPlayerState* BlackoutPlayerState = PlayerState.Get())
	{
		if (BoundPlayerState.Get() != BlackoutPlayerState)
		{
			if (ABlackoutPlayerState* PreviousPlayerState = BoundPlayerState.Get())
			{
				PreviousPlayerState->OnConsumableCountsChanged.RemoveAll(this);
			}

			BlackoutPlayerState->OnConsumableCountsChanged.AddDynamic(this, &UBlackoutHUDWidgetController::HandleConsumablesChanged);
			BoundPlayerState = BlackoutPlayerState;
		}
	}
	else
	{
		BO_LOG_CORE(Warning, "HUD 소모품 바인딩 보류: PlayerState가 유효하지 않습니다.");
	}

	if (UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get())
	{
		if (BoundCombatComponent.Get() != BlackoutCombatComponent)
		{
			if (UBlackoutCombatComponent* PreviousCombatComponent = BoundCombatComponent.Get())
			{
				PreviousCombatComponent->OnEquippedWeaponChanged.RemoveAll(this);
				PreviousCombatComponent->OnAimingChanged.RemoveAll(this);
			}

			BlackoutCombatComponent->OnEquippedWeaponChanged.AddDynamic(this, &UBlackoutHUDWidgetController::HandleEquippedWeaponChanged);
			BlackoutCombatComponent->OnAimingChanged.AddDynamic(this, &UBlackoutHUDWidgetController::HandleAimingChanged);
			BoundCombatComponent = BlackoutCombatComponent;
		}
	}
	else
	{
		BO_LOG_CORE(Warning, "HUD 무기 바인딩 보류: CombatComponent가 유효하지 않습니다.");
	}
}

void UBlackoutHUDWidgetController::BroadcastInitialValues()
{
	BroadcastHealth();
	BroadcastStamina();
	BroadcastAmmo();
	BroadcastEquippedWeapon();
	BroadcastAiming();
	BroadcastWeaponAmmoDisplay(false);
	BroadcastConsumables();
}

bool UBlackoutHUDWidgetController::GetImpactIndicatorData(FBlackoutImpactIndicatorData& OutIndicatorData) const
{
	OutIndicatorData = FBlackoutImpactIndicatorData();

	ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	const UBlackoutImpactIndicatorComponent* BlackoutImpactIndicatorComponent = ImpactIndicatorComponent.Get();
	if (!BlackoutPlayerController || !BlackoutPlayerController->IsLocalController() || !BlackoutImpactIndicatorComponent)
	{
		return false;
	}

	if (!BlackoutImpactIndicatorComponent->GetImpactIndicatorData(OutIndicatorData))
	{
		return false;
	}

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		BlackoutPlayerController,
		OutIndicatorData.WorldLocation,
		ScreenPosition,
		true))
	{
		return false;
	}

	OutIndicatorData.ScreenPosition = ScreenPosition;

	return true;
}

void UBlackoutHUDWidgetController::HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag)
{
	OnEquippedWeaponChanged.Broadcast(EquippedWeapon, WeaponSlotTag);
	BroadcastAiming();
	BroadcastAmmo();
	BroadcastWeaponAmmoDisplay(true);
}

void UBlackoutHUDWidgetController::HandleAimingChanged(bool bIsAiming)
{
	OnAimingChanged.Broadcast(bIsAiming, GetEquippedCrosshairType());
}

bool UBlackoutHUDWidgetController::ResolveDependencies(APlayerController* InPlayerController)
{
	ABlackoutPlayerController* BlackoutPlayerController = Cast<ABlackoutPlayerController>(InPlayerController);
	if (!BlackoutPlayerController)
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: PlayerController가 ABlackoutPlayerController가 아닙니다.");
		return false;
	}

	ABlackoutPlayerState* BlackoutPlayerState = BlackoutPlayerController->GetPlayerState<ABlackoutPlayerState>();
	if (!BlackoutPlayerState)
	{
		BO_LOG_CORE(Verbose, "HUD 초기화 보류: ABlackoutPlayerState 복제를 기다리는 중입니다.");
		return false;
	}

	UAbilitySystemComponent* ASC = BlackoutPlayerState->GetAbilitySystemComponent();
	if (!ASC)
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: PlayerState의 AbilitySystemComponent가 유효하지 않습니다.");
		return false;
	}

	PlayerController = BlackoutPlayerController;
	PlayerState = BlackoutPlayerState;
	AbilitySystemComponent = ASC;
	BaseAttributeSet = ASC->GetSet<UBlackoutBaseAttributeSet>();
	PlayerAttributeSet = ASC->GetSet<UBlackoutPlayerAttributeSet>();
	AmmoAttributeSet = ASC->GetSet<UBlackoutAmmoAttributeSet>();

	if (!BaseAttributeSet.IsValid())
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: UBlackoutBaseAttributeSet을 찾을 수 없습니다.");
		return false;
	}

	if (!PlayerAttributeSet.IsValid())
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: UBlackoutPlayerAttributeSet을 찾을 수 없습니다.");
		return false;
	}

	if (!AmmoAttributeSet.IsValid())
	{
		BO_LOG_CORE(Error, "HUD 초기화 실패: UBlackoutAmmoAttributeSet을 찾을 수 없습니다.");
		return false;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(BlackoutPlayerController->GetPawn()))
	{
		CombatComponent = PlayerCharacter->GetCombatComponent();
		ImpactIndicatorComponent = PlayerCharacter->GetImpactIndicatorComponent();
	}

	if (!CombatComponent.IsValid())
	{
		BO_LOG_CORE(Warning, "HUD 초기화: CombatComponent를 아직 찾을 수 없습니다. 무기 UI는 재초기화 전까지 갱신되지 않을 수 있습니다.");
	}

	if (!ImpactIndicatorComponent.IsValid())
	{
		BO_LOG_CORE(Warning, "HUD 초기화: ImpactIndicatorComponent를 아직 찾을 수 없습니다. 착탄 인디케이터는 재초기화 전까지 갱신되지 않을 수 있습니다.");
	}

	return true;
}

void UBlackoutHUDWidgetController::BroadcastHealth() const
{
	OnHealthChanged.Broadcast(
		GetAttributeValue(UBlackoutBaseAttributeSet::GetHealthAttribute()),
		GetAttributeValue(UBlackoutBaseAttributeSet::GetMaxHealthAttribute()));
}

void UBlackoutHUDWidgetController::BroadcastStamina() const
{
	OnStaminaChanged.Broadcast(
		GetAttributeValue(UBlackoutPlayerAttributeSet::GetStaminaAttribute()),
		GetAttributeValue(UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute()));
}

void UBlackoutHUDWidgetController::BroadcastAmmo() const
{
	const bool bUseSecondaryAmmo = GetEquippedWeaponSlotTag().MatchesTagExact(BlackoutGameplayTags::Weapon_Secondary);

	const int32 ClipAmmo = FMath::RoundToInt(GetAttributeValue(
		bUseSecondaryAmmo
			? UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute()
			: UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute()));
	const int32 MaxClipAmmo = FMath::RoundToInt(GetAttributeValue(
		bUseSecondaryAmmo
			? UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute()
			: UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute()));
	const int32 ReserveAmmo = FMath::RoundToInt(GetAttributeValue(
		bUseSecondaryAmmo
			? UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute()
			: UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute()));

	OnAmmoChanged.Broadcast(ClipAmmo, MaxClipAmmo, ReserveAmmo);
}

void UBlackoutHUDWidgetController::BroadcastEquippedWeapon() const
{
	ABOWeaponBase* EquippedWeapon = nullptr;
	if (const UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get())
	{
		EquippedWeapon = BlackoutCombatComponent->GetEquippedWeapon();
	}

	OnEquippedWeaponChanged.Broadcast(EquippedWeapon, GetEquippedWeaponSlotTag());
}

void UBlackoutHUDWidgetController::BroadcastAiming() const
{
	const UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get();
	OnAimingChanged.Broadcast(BlackoutCombatComponent ? BlackoutCombatComponent->IsAiming() : false, GetEquippedCrosshairType());
}

void UBlackoutHUDWidgetController::BroadcastWeaponAmmoDisplay(bool bPlaySwapAnimation) const
{
	const UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get();
	if (!BlackoutCombatComponent)
	{
		OnWeaponAmmoDisplayChanged.Broadcast(
			FBlackoutWeaponAmmoSlotData(),
			FBlackoutWeaponAmmoSlotData(),
			false);
		return;
	}

	const FGameplayTag EquippedWeaponSlotTag = BlackoutCombatComponent->GetEquippedWeaponSlotTag();
	const FBlackoutWeaponAmmoSlotData PrimarySlotData = MakeWeaponAmmoSlotData(
		BlackoutCombatComponent->GetPrimaryWeapon(),
		BlackoutGameplayTags::Weapon_Primary,
		EquippedWeaponSlotTag.MatchesTagExact(BlackoutGameplayTags::Weapon_Primary));
	const FBlackoutWeaponAmmoSlotData SecondarySlotData = MakeWeaponAmmoSlotData(
		BlackoutCombatComponent->GetSecondaryWeapon(),
		BlackoutGameplayTags::Weapon_Secondary,
		EquippedWeaponSlotTag.MatchesTagExact(BlackoutGameplayTags::Weapon_Secondary));

	OnWeaponAmmoDisplayChanged.Broadcast(PrimarySlotData, SecondarySlotData, bPlaySwapAnimation);
}

void UBlackoutHUDWidgetController::BroadcastConsumables() const
{
	const ABlackoutPlayerState* BlackoutPlayerState = PlayerState.Get();
	if (!BlackoutPlayerState)
	{
		BO_LOG_CORE(Error, "HUD 소모품 조회 실패: PlayerState가 유효하지 않습니다.");
		OnConsumablesChanged.Broadcast(0, 0);
		OnConsumableSlotsChanged.Broadcast(FBlackoutConsumableSlotData(), FBlackoutConsumableSlotData());
		return;
	}

	OnConsumablesChanged.Broadcast(BlackoutPlayerState->BloodRootCount, BlackoutPlayerState->GulSerumCount);
	BroadcastConsumableSlots(BlackoutPlayerState->BloodRootCount, BlackoutPlayerState->GulSerumCount);
}

void UBlackoutHUDWidgetController::BroadcastConsumableSlots(int32 BloodRootCount, int32 GulSerumCount) const
{
	FBlackoutConsumableSlotData BloodRootData;
	FBlackoutConsumableSlotData GulSerumData;
	BloodRootData.CurrentCount = BloodRootCount;
	GulSerumData.CurrentCount = GulSerumCount;

	const ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	const ABlackoutPlayerCharacter* PlayerCharacter = BlackoutPlayerController ? Cast<ABlackoutPlayerCharacter>(BlackoutPlayerController->GetPawn()) : nullptr;
	const UBOCharacterData* CharacterData = PlayerCharacter ? PlayerCharacter->GetCharacterData() : nullptr;
	if (!CharacterData)
	{
		BO_LOG_CORE(Warning, "HUD 소모품 아이콘 조회 실패: CharacterData가 유효하지 않습니다.");
		OnConsumableSlotsChanged.Broadcast(BloodRootData, GulSerumData);
		return;
	}

	if (CharacterData->ConsumableSlots.IsValidIndex(0))
	{
		BloodRootData = MakeConsumableSlotData(CharacterData->ConsumableSlots[0]);
	}

	if (CharacterData->ConsumableSlots.IsValidIndex(1))
	{
		GulSerumData = MakeConsumableSlotData(CharacterData->ConsumableSlots[1]);
	}

	OnConsumableSlotsChanged.Broadcast(BloodRootData, GulSerumData);
}

FBlackoutWeaponAmmoSlotData UBlackoutHUDWidgetController::MakeWeaponAmmoSlotData(ABOWeaponBase* Weapon, FGameplayTag WeaponSlotTag, bool bIsEquipped) const
{
	FBlackoutWeaponAmmoSlotData SlotData;
	SlotData.Weapon = Weapon;
	SlotData.WeaponIcon = Weapon ? Weapon->GetWeaponIcon() : nullptr;
	SlotData.WeaponSlotTag = WeaponSlotTag;
	SlotData.bIsEquipped = bIsEquipped;
	SlotData.bIsValid = IsValid(Weapon);

	if (!SlotData.bIsValid)
	{
		return SlotData;
	}

	const bool bIsPrimary = WeaponSlotTag.MatchesTagExact(BlackoutGameplayTags::Weapon_Primary);
	const bool bIsSecondary = WeaponSlotTag.MatchesTagExact(BlackoutGameplayTags::Weapon_Secondary);
	SlotData.bUsesAmmo = bIsPrimary || bIsSecondary;
	if (!SlotData.bUsesAmmo)
	{
		return SlotData;
	}

	if (bIsSecondary)
	{
		SlotData.CurrentAmmo = FMath::RoundToInt(GetAttributeValue(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute()));
		SlotData.ReserveAmmo = FMath::RoundToInt(GetAttributeValue(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute()));
	}
	else
	{
		SlotData.CurrentAmmo = FMath::RoundToInt(GetAttributeValue(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute()));
		SlotData.ReserveAmmo = FMath::RoundToInt(GetAttributeValue(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute()));
	}

	SlotData.TotalAmmo = SlotData.CurrentAmmo + SlotData.ReserveAmmo;
	return SlotData;
}

FBlackoutConsumableSlotData UBlackoutHUDWidgetController::MakeConsumableSlotData(UBOConsumableData* ConsumableData) const
{
	FBlackoutConsumableSlotData SlotData;
	SlotData.ConsumableData = ConsumableData;
	SlotData.bIsValid = IsValid(ConsumableData);

	if (!SlotData.bIsValid)
	{
		return SlotData;
	}

	SlotData.ConsumableTag = ConsumableData->ConsumableTag;
	SlotData.CurrentCount = PlayerState.IsValid() ? PlayerState->GetConsumableCount(ConsumableData->ConsumableTag) : 0;
	SlotData.MaxCount = ConsumableData->MaxCount;
	SlotData.Icon = ConsumableData->Icon.LoadSynchronous();

	return SlotData;
}

float UBlackoutHUDWidgetController::GetAttributeValue(const FGameplayAttribute& Attribute) const
{
	const UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		BO_LOG_CORE(Error, "HUD Attribute 조회 실패: AbilitySystemComponent가 유효하지 않습니다.");
		return 0.0f;
	}

	return ASC->GetNumericAttribute(Attribute);
}

FGameplayTag UBlackoutHUDWidgetController::GetEquippedWeaponSlotTag() const
{
	if (const UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get())
	{
		return BlackoutCombatComponent->GetEquippedWeaponSlotTag();
	}

	return BlackoutGameplayTags::Weapon_Primary;
}

int32 UBlackoutHUDWidgetController::GetEquippedCrosshairType() const
{
	const UBlackoutCombatComponent* BlackoutCombatComponent = CombatComponent.Get();
	const ABOWeaponBase* EquippedWeapon = BlackoutCombatComponent ? BlackoutCombatComponent->GetEquippedWeapon() : nullptr;
	return EquippedWeapon ? EquippedWeapon->GetCrosshairType() : 0;
}

void UBlackoutHUDWidgetController::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastHealth();
}

void UBlackoutHUDWidgetController::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastHealth();
}

void UBlackoutHUDWidgetController::HandleStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastStamina();
}

void UBlackoutHUDWidgetController::HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastStamina();
}

void UBlackoutHUDWidgetController::HandleAmmoChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastAmmo();
	BroadcastWeaponAmmoDisplay(false);
}

void UBlackoutHUDWidgetController::HandleConsumablesChanged(int32 BloodRootCount, int32 GulSerumCount)
{
	OnConsumablesChanged.Broadcast(BloodRootCount, GulSerumCount);
	BroadcastConsumableSlots(BloodRootCount, GulSerumCount);
}
