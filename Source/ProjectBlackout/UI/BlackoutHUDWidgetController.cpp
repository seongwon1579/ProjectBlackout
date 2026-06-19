#include "UI/BlackoutHUDWidgetController.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "Core/BlackoutLog.h"
#include "Data/BOCharacterData.h"
#include "Data/BOConsumableData.h"
#include "EngineUtils.h"
#include "Framework/BlackoutPlayerController.h"
#include "Framework/BlackoutPlayerState.h"
#include "Framework/BlackoutGameState.h"
#include "GAS/Abilities/Player/BlackoutGA_Revive.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutInteractable.h"
#include "Blueprint/WidgetLayoutLibrary.h"

namespace
{
	const FText DefaultInteractionPromptText = FText::FromString(TEXT("상호작용"));
	const FText RevivePromptText = FText::FromString(TEXT("부활"));
	const FText MissingRelicText = FText::FromString(TEXT("유물이 없습니다"));
	const FText ReviveBusyText = FText::FromString(TEXT("부활중입니다"));
	const FText ReviveInProgressText = FText::FromString(TEXT("소생 중..."));

	FText ResolveInteractionPromptText(AActor* FocusedInteractableActor)
	{
		if (!FocusedInteractableActor ||
			!FocusedInteractableActor->GetClass()->ImplementsInterface(UBlackoutInteractable::StaticClass()))
		{
			return DefaultInteractionPromptText;
		}

		const FText PromptText = IBlackoutInteractable::Execute_GetInteractionPrompt(FocusedInteractableActor);
		return PromptText.IsEmpty() ? DefaultInteractionPromptText : PromptText;
	}

	FVector GetRevivePromptWorldLocation(const ABlackoutPlayerCharacter* TargetCharacter)
	{
		if (!TargetCharacter)
		{
			return FVector::ZeroVector;
		}

		// 머리 위보다 약간 높은 지점에 프롬프트를 고정해 다운된 아군 위치를 쉽게 읽도록 합니다.
		return TargetCharacter->GetActorLocation() +
			FVector(0.0f, 0.0f, TargetCharacter->GetSimpleCollisionHalfHeight());
	}
}

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
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleRelicChargesChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutPlayerAttributeSet::GetMaxRelicChargesAttribute())
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleMaxRelicChargesChanged);
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

	if (ASC && BoundStateTagAbilitySystemComponent.Get() != ASC)
	{
		// 이전 ASC에 남아있던 State 태그 콜백을 정리합니다.
		if (UAbilitySystemComponent* PreviousASC = BoundStateTagAbilitySystemComponent.Get())
		{
			if (DownedTagChangedHandle.IsValid())
			{
				PreviousASC->RegisterGameplayTagEvent(
					BlackoutGameplayTags::State_Downed,
					EGameplayTagEventType::NewOrRemoved).Remove(DownedTagChangedHandle);
			}
			if (BeingRevivedTagChangedHandle.IsValid())
			{
				PreviousASC->RegisterGameplayTagEvent(
					BlackoutGameplayTags::State_BeingRevived,
					EGameplayTagEventType::NewOrRemoved).Remove(BeingRevivedTagChangedHandle);
			}
			if (DeadTagChangedHandle.IsValid())
			{
				PreviousASC->RegisterGameplayTagEvent(
					BlackoutGameplayTags::State_Dead,
					EGameplayTagEventType::NewOrRemoved).Remove(DeadTagChangedHandle);
			}
		}

		DownedTagChangedHandle = ASC->RegisterGameplayTagEvent(
			BlackoutGameplayTags::State_Downed,
			EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleDownedRelatedTagChanged);

		BeingRevivedTagChangedHandle = ASC->RegisterGameplayTagEvent(
			BlackoutGameplayTags::State_BeingRevived,
			EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleDownedRelatedTagChanged);

		DeadTagChangedHandle = ASC->RegisterGameplayTagEvent(
			BlackoutGameplayTags::State_Dead,
			EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UBlackoutHUDWidgetController::HandleDownedRelatedTagChanged);

		BoundStateTagAbilitySystemComponent = ASC;
	}

	if (ASC && BoundCooldownAbilitySystemComponent.Get() != ASC)
	{
		// 이전 ASC에 맺어두었던 쿨다운 변경 델리게이트를 안전하게 해제합니다.
		if (UBlackoutAbilitySystemComponent* PreviousBlackoutASC = Cast<UBlackoutAbilitySystemComponent>(BoundCooldownAbilitySystemComponent.Get()))
		{
			if (ConsumableCooldownChangedHandle.IsValid())
			{
				PreviousBlackoutASC->OnConsumableCooldownChanged.Remove(ConsumableCooldownChangedHandle);
			}
		}

		// 새 ASC의 쿨다운 변경 델리게이트를 구독합니다.
		if (UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(ASC))
		{
			ConsumableCooldownChangedHandle = BlackoutASC->OnConsumableCooldownChanged.AddUObject(
				this, &UBlackoutHUDWidgetController::HandleConsumableCooldownChanged);
			BoundCooldownAbilitySystemComponent = ASC;
		}
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

			// 캐릭터 교체로 새 전투 컴포넌트에 바인딩되면 새 캐릭터는 항상 주무기를 장착한다.
			// 무기 슬롯 UI는 스왑 애니메이션 위치로 장착 슬롯을 표현하고 위젯 인스턴스는 유지되므로,
			// 직전 표시가 보조무기였다면 주무기 위치로 스왑 애니메이션을 강제 재생해 정렬한다.
			// (직전이 이미 주무기였다면 애니메이션이 그대로이므로 불필요한 재생/글리치를 피한다.)
			if (!bLastDisplayedPrimaryEquipped)
			{
				BroadcastWeaponAmmoDisplay(true, BlackoutGameplayTags::Weapon_Primary);
			}
		}
	}
	else
	{
		BO_LOG_CORE(Warning, "HUD 무기 바인딩 보류: CombatComponent가 유효하지 않습니다.");
	}

	if (UWorld* World = GetWorld())
	{
		if (ABlackoutGameState* GS = World->GetGameState<ABlackoutGameState>())
		{
			GS->OnSurrenderVoteStateChanged.AddUniqueDynamic(this, &UBlackoutHUDWidgetController::HandleSurrenderVoteStateChanged);
		}
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
	BroadcastRelicCharges();
	BroadcastConsumables();

	// HUD 모드와 다운 상태 데이터의 초기값을 브로드캐스트해 첫 프레임에 위젯 가시성이 결정되도록 합니다.
	CurrentHUDMode = EvaluateHUDMode();
	OnHUDModeChanged.Broadcast(CurrentHUDMode);
	OnDownedStateHUDDataChanged.Broadcast(BuildDownedStateHUDData(CurrentHUDMode));

	if (UWorld* World = GetWorld())
	{
		if (const ABlackoutGameState* GS = World->GetGameState<ABlackoutGameState>())
		{
			OnSurrenderVoteStateChanged.Broadcast(
				GS->bIsSurrenderVoteActive,
				GS->SurrenderVoteYesCount,
				GS->SurrenderVoteNoCount,
				GS->SurrenderVoteEndTimeSeconds
			);
		}
	}
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
		OutIndicatorData.bIsVisible = false;
		OutIndicatorData.TrajectoryPoints.Reset();
		return false;
	}

	OutIndicatorData.ScreenPosition = ScreenPosition;

	// 전투 컴포넌트는 월드 좌표만 만들고, 화면 좌표 변환은 HUD 계층에서 처리합니다.
	ProjectTrajectoryPoints(OutIndicatorData.TrajectoryPoints);

	return true;
}

bool UBlackoutHUDWidgetController::GetInteractionPromptData(FBlackoutInteractionPromptData& OutPromptData) const
{
	OutPromptData = FBlackoutInteractionPromptData();

	const ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	const ABlackoutPlayerCharacter* LocalPlayerCharacter =
		BlackoutPlayerController ? Cast<ABlackoutPlayerCharacter>(BlackoutPlayerController->GetPawn()) : nullptr;
	if (!BlackoutPlayerController || !BlackoutPlayerController->IsLocalController() || !LocalPlayerCharacter)
	{
		return false;
	}

	if (LocalPlayerCharacter->IsDead() || LocalPlayerCharacter->IsDowned())
	{
		return false;
	}

	auto BuildReviveInProgressPromptData =
		[this, BlackoutPlayerController, &OutPromptData](const ABlackoutPlayerCharacter* ActiveReviveTarget, float ProgressNormalized)
	{
		if (!ActiveReviveTarget)
		{
			return false;
		}

		const FVector PromptWorldLocation = GetRevivePromptWorldLocation(ActiveReviveTarget);
		FVector2D PromptScreenPosition = FVector2D::ZeroVector;
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			BlackoutPlayerController,
			PromptWorldLocation,
			PromptScreenPosition,
			true);

		// 진행 UI는 화면 고정 위젯이므로 월드 좌표 투영 실패와 무관하게 표시합니다.
		OutPromptData.bIsVisible = true;
		OutPromptData.bShowProgress = true;
		OutPromptData.ProgressNormalized = FMath::Clamp(ProgressNormalized, 0.0f, 1.0f);
		OutPromptData.WorldLocation = PromptWorldLocation;
		OutPromptData.ScreenPosition = PromptScreenPosition;
		OutPromptData.State = EBlackoutInteractionPromptState::InProgress;
		OutPromptData.PromptText = FText::GetEmpty();
		OutPromptData.StatusText = ReviveInProgressText;
		return true;
	};

	// 데디케이트 서버 환경에서는 서버의 GA 인스턴스가 실제 진행을 소유하므로,
	// 시전자 HUD는 복제된 대상 포인터와 대상의 서버 시간 데이터를 우선 사용합니다.
	if (const ABlackoutPlayerCharacter* ReplicatedReviveTarget = LocalPlayerCharacter->GetActiveReviveTarget())
	{
		if (ReplicatedReviveTarget->IsDowned() && ReplicatedReviveTarget->IsBeingRevived())
		{
			return BuildReviveInProgressPromptData(
				ReplicatedReviveTarget,
				ReplicatedReviveTarget->GetReviveProgressNormalized());
		}
	}

	if (const UBlackoutGA_Revive* ActiveReviveAbility = UBlackoutGA_Revive::GetActiveReviveAbilityFromActor(LocalPlayerCharacter))
	{
		if (ABlackoutPlayerCharacter* ActiveReviveTarget = ActiveReviveAbility->GetReviveTarget())
		{
			return BuildReviveInProgressPromptData(
				ActiveReviveTarget,
				ActiveReviveAbility->GetReviveProgressNormalized());
		}
	}

	const float ReviveRange = UBlackoutGA_Revive::GetReviveRangeForActor(LocalPlayerCharacter);
	ABlackoutPlayerCharacter* NearbyDownedPlayer = FindNearbyDownedPlayer(LocalPlayerCharacter, ReviveRange);
	if (!NearbyDownedPlayer)
	{
		if (AActor* FocusedInteractableActor = LocalPlayerCharacter->GetFocusedInteractableActor())
		{
			const FVector PromptWorldLocation = LocalPlayerCharacter->GetFocusedInteractablePromptWorldLocation();
			FVector2D PromptScreenPosition = FVector2D::ZeroVector;
			if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
				BlackoutPlayerController,
				PromptWorldLocation,
				PromptScreenPosition,
				true))
			{
				return false;
			}

			OutPromptData.bIsVisible = true;
			OutPromptData.WorldLocation = PromptWorldLocation;
			OutPromptData.ScreenPosition = PromptScreenPosition;
			OutPromptData.State = EBlackoutInteractionPromptState::Available;
			OutPromptData.PromptText = ResolveInteractionPromptText(FocusedInteractableActor);
			return true;
		}

		return false;
	}

	const FVector PromptWorldLocation = GetRevivePromptWorldLocation(NearbyDownedPlayer);
	FVector2D PromptScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		BlackoutPlayerController,
		PromptWorldLocation,
		PromptScreenPosition,
		true))
	{
		return false;
	}

	OutPromptData.bIsVisible = true;
	OutPromptData.WorldLocation = PromptWorldLocation;
	OutPromptData.ScreenPosition = PromptScreenPosition;
	OutPromptData.PromptText = RevivePromptText;

	if (NearbyDownedPlayer->IsBeingRevived())
	{
		OutPromptData.State = EBlackoutInteractionPromptState::Busy;
		OutPromptData.bIsStatusError = true;
		OutPromptData.StatusText = ReviveBusyText;
		return true;
	}

	const int32 CurrentRelicCharges = FMath::Max(
		0,
		FMath::RoundToInt(GetAttributeValue(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute())));
	if (CurrentRelicCharges <= 0)
	{
		OutPromptData.State = EBlackoutInteractionPromptState::MissingRequirement;
		OutPromptData.bIsStatusError = true;
		OutPromptData.StatusText = MissingRelicText;
		return true;
	}

	OutPromptData.State = EBlackoutInteractionPromptState::Available;
	return true;
}

void UBlackoutHUDWidgetController::HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag)
{
	OnEquippedWeaponChanged.Broadcast(EquippedWeapon, WeaponSlotTag);
	BroadcastAiming();
	BroadcastAmmo();
	// 이벤트가 전달한 슬롯 태그를 그대로 사용한다(무기 포인터 비교 재계산으로 인한 오판정 방지).
	BroadcastWeaponAmmoDisplay(true, WeaponSlotTag);
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

void UBlackoutHUDWidgetController::BroadcastWeaponAmmoDisplay(bool bPlaySwapAnimation, FGameplayTag EquippedSlotTagOverride)
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

	// 호출부가 슬롯 태그를 명시하면 그것을 신뢰한다(클래스 변경 직후 등 무기 포인터 비교가 불안정한 시점 대응).
	const FGameplayTag EquippedWeaponSlotTag = EquippedSlotTagOverride.IsValid()
		? EquippedSlotTagOverride
		: BlackoutCombatComponent->GetEquippedWeaponSlotTag();

	// 스왑 애니메이션을 재생하는 경우에만 위젯의 표시 슬롯이 바뀌므로 이때만 추적 값을 갱신한다.
	if (bPlaySwapAnimation)
	{
		bLastDisplayedPrimaryEquipped = EquippedWeaponSlotTag.MatchesTagExact(BlackoutGameplayTags::Weapon_Primary);
	}

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

void UBlackoutHUDWidgetController::BroadcastRelicCharges() const
{
	const int32 MaxCharges = FMath::Max(0, FMath::RoundToInt(GetAttributeValue(UBlackoutPlayerAttributeSet::GetMaxRelicChargesAttribute())));
	const int32 CurrentCharges = FMath::Clamp(
		FMath::RoundToInt(GetAttributeValue(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute())),
		0,
		MaxCharges);

	OnRelicChargesChanged.Broadcast(CurrentCharges, MaxCharges);
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

	// ASC가 유효할 경우 소모품 어빌리티의 실시간 쿨다운 정보(남은 시간, 총 지속 시간)를 쿼리하여 반영합니다.
	if (UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent.Get()))
	{
		float Remaining = 0.0f;
		float Duration = 0.0f;
		BlackoutASC->GetConsumableCooldownInfo(ConsumableData->ConsumableTag, Remaining, Duration);
		SlotData.CooldownRemaining = Remaining;
		SlotData.CooldownDuration = Duration;
	}

	return SlotData;
}

ABlackoutPlayerCharacter* UBlackoutHUDWidgetController::FindNearbyDownedPlayer(
	const ABlackoutPlayerCharacter* LocalPlayerCharacter,
	float ReviveRange) const
{
	if (!LocalPlayerCharacter || !LocalPlayerCharacter->GetWorld())
	{
		return nullptr;
	}

	const float MaxRangeSquared = FMath::Square(FMath::Max(0.0f, ReviveRange));
	ABlackoutPlayerCharacter* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<ABlackoutPlayerCharacter> It(LocalPlayerCharacter->GetWorld()); It; ++It)
	{
		ABlackoutPlayerCharacter* Candidate = *It;
		if (!Candidate || Candidate == LocalPlayerCharacter || Candidate->IsDead() || !Candidate->IsDowned())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			LocalPlayerCharacter->GetActorLocation(),
			Candidate->GetActorLocation());
		if (DistanceSquared > MaxRangeSquared || DistanceSquared >= BestDistanceSquared)
		{
			continue;
		}

		BestDistanceSquared = DistanceSquared;
		BestTarget = Candidate;
	}

	return BestTarget;
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

void UBlackoutHUDWidgetController::ProjectTrajectoryPoints(TArray<FBlackoutTrajectoryPointData>& TrajectoryPoints) const
{
	ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	if (!BlackoutPlayerController || TrajectoryPoints.Num() <= 0)
	{
		TrajectoryPoints.Reset();
		return;
	}

	for (int32 PointIndex = TrajectoryPoints.Num() - 1; PointIndex >= 0; --PointIndex)
	{
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			BlackoutPlayerController,
			TrajectoryPoints[PointIndex].WorldLocation,
			ScreenPosition,
			true))
		{
			// 화면 좌표로 투영할 수 없는 포인트는 점 렌더링 대상에서 제외합니다.
			TrajectoryPoints.RemoveAt(PointIndex);
			continue;
		}

		TrajectoryPoints[PointIndex].ScreenPosition = ScreenPosition;
	}
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

void UBlackoutHUDWidgetController::HandleRelicChargesChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastRelicCharges();
}

void UBlackoutHUDWidgetController::HandleMaxRelicChargesChanged(const FOnAttributeChangeData& ChangeData)
{
	BroadcastRelicCharges();
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

void UBlackoutHUDWidgetController::HandleConsumableCooldownChanged(FGameplayTag ConsumableTag)
{
	// 쿨다운 상태가 변했을 때(시작 또는 리셋) 소모품 슬롯 데이터를 다시 빌드하여 UI 위젯으로 브로드캐스트합니다.
	BroadcastConsumables();
}

bool UBlackoutHUDWidgetController::GetDownedStateHUDData(FBlackoutDownedStateHUDData& OutHUDData) const
{
	OutHUDData = BuildDownedStateHUDData(CurrentHUDMode);
	return OutHUDData.bIsVisible;
}

bool UBlackoutHUDWidgetController::GetSpectatorTargetName(FText& OutTargetName) const
{
	OutTargetName = FText::GetEmpty();

	if (CurrentHUDMode != EBlackoutHUDMode::Spectator)
	{
		return false;
	}

	const ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	if (!BlackoutPlayerController)
	{
		return false;
	}

	// ViewTarget의 PlayerState 닉네임을 가져옵니다. 본인 시점이거나 PS가 없으면 표시할 이름이 없습니다.
	const AActor* CurrentViewTarget = BlackoutPlayerController->GetViewTarget();
	const APawn* ViewTargetPawn = Cast<APawn>(CurrentViewTarget);
	if (!ViewTargetPawn || ViewTargetPawn == BlackoutPlayerController->GetPawn())
	{
		return false;
	}

	if (const APlayerState* TargetPlayerState = ViewTargetPawn->GetPlayerState())
	{
		OutTargetName = FText::FromString(TargetPlayerState->GetPlayerName());
		return !OutTargetName.IsEmpty();
	}

	return false;
}

void UBlackoutHUDWidgetController::HandleDownedRelatedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 태그 콜백은 NewCount만 알려주므로 현재 ASC 태그 컨테이너 전체를 다시 평가합니다.
	RefreshHUDMode();
}

void UBlackoutHUDWidgetController::RefreshHUDMode()
{
	const EBlackoutHUDMode NewMode = EvaluateHUDMode();
	const FBlackoutDownedStateHUDData NewData = BuildDownedStateHUDData(NewMode);

	if (NewMode != CurrentHUDMode)
	{
		CurrentHUDMode = NewMode;
		OnHUDModeChanged.Broadcast(CurrentHUDMode);
	}

	// 모드가 그대로여도 데이터는 변화할 수 있으므로 항상 브로드캐스트합니다(StatusText/시간 등).
	OnDownedStateHUDDataChanged.Broadcast(NewData);
}

EBlackoutHUDMode UBlackoutHUDWidgetController::EvaluateHUDMode() const
{
	const UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		return EBlackoutHUDMode::Combat;
	}

	if (ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead))
	{
		return EBlackoutHUDMode::Spectator;
	}

	if (ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed))
	{
		return ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_BeingRevived)
			? EBlackoutHUDMode::DownedReviveTimer
			: EBlackoutHUDMode::DownedDeathTimer;
	}

	return EBlackoutHUDMode::Combat;
}

FBlackoutDownedStateHUDData UBlackoutHUDWidgetController::BuildDownedStateHUDData(EBlackoutHUDMode InHUDMode) const
{
	FBlackoutDownedStateHUDData HUDData;
	HUDData.HUDMode = InHUDMode;

	const ABlackoutPlayerController* BlackoutPlayerController = PlayerController.Get();
	const ABlackoutPlayerCharacter* LocalPlayerCharacter =
		BlackoutPlayerController ? Cast<ABlackoutPlayerCharacter>(BlackoutPlayerController->GetPawn()) : nullptr;

	switch (InHUDMode)
	{
	case EBlackoutHUDMode::DownedDeathTimer:
	{
		if (!LocalPlayerCharacter)
		{
			break;
		}

		const float Total = FMath::Max(KINDA_SMALL_NUMBER, LocalPlayerCharacter->GetDownedDeathDuration());
		const float Remaining = FMath::Clamp(LocalPlayerCharacter->GetDownedDeathRemainingTime(), 0.0f, Total);

		HUDData.TotalDuration = Total;
		HUDData.RemainingTime = Remaining;
		// 사망 타이머는 남은 시간이 줄어들수록 게이지가 차오르는 카운트다운 형태로 표시합니다.
		HUDData.ProgressNormalized = FMath::Clamp(1.0f - (Remaining / Total), 0.0f, 1.0f);
		HUDData.bIsVisible = true;
		break;
	}
	case EBlackoutHUDMode::DownedReviveTimer:
	{
		if (!LocalPlayerCharacter)
		{
			break;
		}

		const float ReviveTotal = FMath::Max(KINDA_SMALL_NUMBER, LocalPlayerCharacter->GetReviveDuration());
		const float ReviveRemaining = FMath::Clamp(LocalPlayerCharacter->GetReviveRemainingTime(), 0.0f, ReviveTotal);

		HUDData.TotalDuration = ReviveTotal;
		HUDData.RemainingTime = ReviveRemaining;
		// 위젯이 SetPercent 시 (1 - ProgressNormalized)를 적용하므로,
		// 부활 진행 바가 0→1로 차오르도록 보조 값(= 남은 비율)을 전달합니다.
		HUDData.ProgressNormalized = FMath::Clamp(ReviveRemaining / ReviveTotal, 0.0f, 1.0f);
		HUDData.bIsVisible = true;
		break;
	}
	case EBlackoutHUDMode::Spectator:
		// 관전 HUD 상세는 후속 문서에서 정의되므로 여기서는 다운 위젯을 노출하지 않습니다.
		HUDData.bIsVisible = false;
		break;
	case EBlackoutHUDMode::Combat:
	default:
		HUDData.bIsVisible = false;
		break;
	}

	return HUDData;
}

void UBlackoutHUDWidgetController::HandleSurrenderVoteStateChanged(bool bIsActive, int32 YesCount, int32 NoCount, float EndTimeSeconds)
{
	OnSurrenderVoteStateChanged.Broadcast(bIsActive, YesCount, NoCount, EndTimeSeconds);
}
