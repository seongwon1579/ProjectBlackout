#include "BlackoutAbilitySystemComponent.h"

#include "Abilities/BlackoutGameplayAbility.h"
#include "BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UBlackoutAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBlackoutAbilitySystemComponent, StaminaCostMultiplier);
}

void UBlackoutAbilitySystemComponent::GiveDefaultAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1);
		
		// 만약 UBlackoutGameplayAbility라면 정의된 InputID를 Spec에 할당
		if (const UBlackoutGameplayAbility* BlackoutAbility = Cast<UBlackoutGameplayAbility>(AbilityClass->GetDefaultObject()))
		{
			if (BlackoutAbility->InputID != EBlackoutAbilityInputID::None)
			{
				Spec.InputID = static_cast<int32>(BlackoutAbility->InputID);
			}
		}

		GiveAbility(Spec);
		BO_LOG_GAS(Verbose, "Granted ability: %s (InputID: %d)", *AbilityClass->GetName(), Spec.InputID);
	}
}

void UBlackoutAbilitySystemComponent::GiveConsumableAbilities(const TArray<TObjectPtr<UBOConsumableData>>& ConsumableSlots)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < ConsumableSlots.Num(); ++SlotIndex)
	{
		UBOConsumableData* ConsumableData = ConsumableSlots[SlotIndex];
		if (!ConsumableData || !ConsumableData->UseAbility)
		{
			BO_LOG_GAS(Warning, "소모품 어빌리티 부여 실패: Slot=%d Data=%s UseAbility=%s",
				SlotIndex,
				*GetNameSafe(ConsumableData),
				ConsumableData ? *GetNameSafe(ConsumableData->UseAbility.Get()) : TEXT("None"));
			continue;
		}

		FGameplayAbilitySpec Spec(ConsumableData->UseAbility, 1);
		Spec.SourceObject = ConsumableData;

		if (SlotIndex == 0)
		{
			Spec.InputID = static_cast<int32>(EBlackoutAbilityInputID::UseConsumable1);
		}
		else if (SlotIndex == 1)
		{
			Spec.InputID = static_cast<int32>(EBlackoutAbilityInputID::UseConsumable2);
		}
		else if (const UBlackoutGameplayAbility* BlackoutAbility = Cast<UBlackoutGameplayAbility>(ConsumableData->UseAbility->GetDefaultObject()))
		{
			Spec.InputID = static_cast<int32>(BlackoutAbility->InputID);
		}

		GiveAbility(Spec);
		BO_LOG_GAS(Log, "Granted consumable ability: Slot=%d Data=%s Ability=%s InputID=%d",
			SlotIndex,
			*GetNameSafe(ConsumableData),
			*GetNameSafe(Spec.Ability),
			Spec.InputID);
	}
}

void UBlackoutAbilitySystemComponent::HandleAbilityInputPressed(EBlackoutAbilityInputID InputID)
{
	if (InputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	const int32 RawInputID = static_cast<int32>(InputID);
	bool bFoundMatchingAbility = false;

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID != RawInputID)
		{
			continue;
		}

		bFoundMatchingAbility = true;
		AbilitySpec.InputPressed = true;

		if (AbilitySpec.IsActive())
		{
			if (!IsOwnerActorAuthoritative())
			{
				ServerSetInputPressed(AbilitySpec.Handle);
			}

			AbilitySpecInputPressed(AbilitySpec);
			//InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			continue;
		}

		const bool bActivated = TryActivateAbility(AbilitySpec.Handle);
		BO_LOG_GAS(Verbose, "InputPressed: ID=%d Ability=%s Activated=%s", RawInputID, *GetNameSafe(AbilitySpec.Ability), bActivated ? TEXT("true") : TEXT("false"));
	}

	if (!bFoundMatchingAbility)
	{
		BO_LOG_GAS(Verbose, "InputPressed: ID=%d has no matching ability on %s", RawInputID, *GetNameSafe(GetOwner()));
	}
}

void UBlackoutAbilitySystemComponent::HandleAbilityInputReleased(EBlackoutAbilityInputID InputID)
{
	if (InputID == EBlackoutAbilityInputID::None)
	{
		return;
	}

	const int32 RawInputID = static_cast<int32>(InputID);

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID != RawInputID)
		{
			continue;
		}

		AbilitySpec.InputPressed = false;

		if (AbilitySpec.IsActive())
		{
			if (!IsOwnerActorAuthoritative())
			{
				ServerSetInputReleased(AbilitySpec.Handle);
			}

			AbilitySpecInputReleased(AbilitySpec);
			//InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UBlackoutAbilitySystemComponent::ClearAllAbilitiesAndEffects()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	StopStaminaRegen();
	ClearStaminaCostMultiplier();
	StopHealthRegen();
	ClearAllAbilities();

	// 모든 활성 GE 제거 (풀 반환 시 상태이상/쿨다운 초기화)
	RemoveActiveEffects(FGameplayEffectQuery());
	BO_LOG_GAS(Verbose, "ASC cleared: %s", *GetOwner()->GetName());
}

void UBlackoutAbilitySystemComponent::NotifyStaminaSpent()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	StopStaminaRegen();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		StaminaRegenDelayTimerHandle,
		this,
		&UBlackoutAbilitySystemComponent::StartStaminaRegen,
		FMath::Max(0.0f, StaminaRegenDelay),
		false);
}

void UBlackoutAbilitySystemComponent::ApplyTemporaryStaminaCostMultiplier(float NewMultiplier, float Duration)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	StaminaCostMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 1.0f);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaCostMultiplierTimerHandle);

		if (Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				StaminaCostMultiplierTimerHandle,
				this,
				&UBlackoutAbilitySystemComponent::ClearStaminaCostMultiplier,
				Duration,
				false);
		}
	}

	BO_LOG_GAS(Log, "스태미나 소비 배율 적용: Owner=%s Multiplier=%.2f Duration=%.2f",
		*GetNameSafe(GetOwner()),
		StaminaCostMultiplier,
		Duration);
}

void UBlackoutAbilitySystemComponent::ApplyHealthRegenOverTime(float HealAmountPerTick, float Duration, float TickInterval)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	if (HealAmountPerTick <= 0.0f)
	{
		BO_LOG_GAS(Warning, "지속 체력 회복 적용 실패: 회복량이 0 이하입니다. Owner=%s Heal=%.2f",
			*GetNameSafe(GetOwner()),
			HealAmountPerTick);
		return;
	}

	StopHealthRegen();

	const float SafeTickInterval = FMath::Max(0.1f, TickInterval);
	const int32 TickCount = Duration > 0.0f ? FMath::Max(1, FMath::CeilToInt(Duration / SafeTickInterval)) : 1;
	HealthRegenAmountPerTick = HealAmountPerTick;
	RemainingHealthRegenTickCount = TickCount;

	HandleHealthRegenTick();
	if (RemainingHealthRegenTickCount <= 0)
	{
		StopHealthRegen();
		return;
	}

	if (Duration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				HealthRegenTimerHandle,
				this,
				&UBlackoutAbilitySystemComponent::HandleHealthRegenTick,
				SafeTickInterval,
				true,
				SafeTickInterval);
		}
		else
		{
			BO_LOG_GAS(Warning, "지속 체력 회복 타이머 시작 실패: World가 유효하지 않습니다. Owner=%s",
				*GetNameSafe(GetOwner()));
			StopHealthRegen();
			return;
		}
	}
	else
	{
		StopHealthRegen();
	}

	BO_LOG_GAS(Log, "지속 체력 회복 적용: Owner=%s TickAmount=%.2f Duration=%.2f TickInterval=%.2f TickCount=%d",
		*GetNameSafe(GetOwner()),
		HealAmountPerTick,
		Duration,
		SafeTickInterval,
		TickCount);
}

void UBlackoutAbilitySystemComponent::StartStaminaRegen()
{
	if (!CanRecoverStamina())
	{
		return;
	}

	HandleStaminaRegenTick();

	if (!CanRecoverStamina())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			StaminaRegenTickTimerHandle,
			this,
			&UBlackoutAbilitySystemComponent::HandleStaminaRegenTick,
			FMath::Max(0.01f, StaminaRegenTickInterval),
			true);
	}
}

void UBlackoutAbilitySystemComponent::HandleStaminaRegenTick()
{
	if (!CanRecoverStamina())
	{
		StopStaminaRegen();
		return;
	}

	const FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(StaminaRegenEffectClass, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		StopStaminaRegen();
		return;
	}

	ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (!CanRecoverStamina())
	{
		StopStaminaRegen();
	}
}

void UBlackoutAbilitySystemComponent::StopStaminaRegen()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaRegenDelayTimerHandle);
		World->GetTimerManager().ClearTimer(StaminaRegenTickTimerHandle);
	}
}

void UBlackoutAbilitySystemComponent::ClearStaminaCostMultiplier()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaCostMultiplierTimerHandle);
	}

	StaminaCostMultiplier = 1.0f;
	BO_LOG_GAS(Log, "스태미나 소비 배율 복구: Owner=%s", *GetNameSafe(GetOwner()));
}

void UBlackoutAbilitySystemComponent::HandleHealthRegenTick()
{
	if (RemainingHealthRegenTickCount <= 0)
	{
		StopHealthRegen();
		return;
	}

	const float CurrentHealth = GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	
	// 체력이 0 이하로 내려가면 중단
	if (MaxHealth <= 0.0f)
	{
		StopHealthRegen();
		return;
	}
	
	// 최대 체력에 도달한 경우 이번 틱은 체력 회복 안 함
	if (CurrentHealth >= MaxHealth)
	{
		return;
	}

	const float HealingEffectivenessAttribute = GetNumericAttribute(UBlackoutPlayerAttributeSet::GetHealingEffectivenessAttribute());
	const float HealingEffectiveness = HealingEffectivenessAttribute > 0.0f ? HealingEffectivenessAttribute : 1.0f;
	const float EffectiveHealAmount = FMath::Min(HealthRegenAmountPerTick * HealingEffectiveness, MaxHealth - CurrentHealth);

	if (EffectiveHealAmount > 0.0f)
	{
		ApplyModToAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, EffectiveHealAmount);
	}

	--RemainingHealthRegenTickCount;
	if (RemainingHealthRegenTickCount <= 0)
	{
		StopHealthRegen();
	}
}

void UBlackoutAbilitySystemComponent::StopHealthRegen()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
	}

	HealthRegenAmountPerTick = 0.0f;
	RemainingHealthRegenTickCount = 0;
}

bool UBlackoutAbilitySystemComponent::CanRecoverStamina() const
{
	if (!IsOwnerActorAuthoritative() || !StaminaRegenEffectClass)
	{
		return false;
	}

	if (HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting))
	{
		return false;
	}

	const float CurrentStamina = GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	const float MaxStamina = GetNumericAttribute(UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute());
	return MaxStamina > 0.0f && CurrentStamina < MaxStamina;
}
