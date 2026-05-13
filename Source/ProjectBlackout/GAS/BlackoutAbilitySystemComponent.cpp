#include "BlackoutAbilitySystemComponent.h"

#include "Abilities/BlackoutGameplayAbility.h"
#include "Abilities/Player/BlackoutGA_UseConsumable.h"
#include "BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
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
	const uint16 InputSequenceId = GenerateNextInputSequence(InputID);
	const float ClientInputTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float ClientEstimatedServerTimeSeconds = GetEstimatedServerTimeSeconds();

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.InputID != RawInputID)
		{
			continue;
		}

		bFoundMatchingAbility = true;
		AbilitySpec.InputPressed = true;

		const FBlackoutAbilityInputSyncPayload InputPayload = BuildInputSyncPayload(
			InputID,
			AbilitySpec.Handle,
			InputSequenceId,
			ClientInputTimeSeconds,
			ClientEstimatedServerTimeSeconds);
		RecordAbilityInputSyncPayload(InputPayload, IsOwnerActorAuthoritative());
		if (!IsOwnerActorAuthoritative())
		{
			Server_RecordAbilityInputSyncPayload(InputPayload);
		}

		if (AbilitySpec.IsActive())
		{
			// 활성 GA의 prediction key 추출.
			// 콤보/체인 재입력은 새 ScopedPredictionWindow 를 만들지 않고 기존 activation key 를 재사용합니다.
			const FGameplayAbilityActivationInfo* ActivationInfo = &AbilitySpec.ActivationInfo;
			const TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
			if (AbilityInstances.Num() > 0 && AbilityInstances.Last())
			{
				ActivationInfo = &AbilityInstances.Last()->GetCurrentActivationInfoRef();
			}
			const FPredictionKey ActivationKey = ActivationInfo->GetActivationPredictionKey();

			// 로컬: 자신의 GA 인스턴스에 InputPressed 를 전달하고 로컬 WaitInputPress 를 발화.
			AbilitySpecInputPressed(AbilitySpec);
			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputPressed,
				AbilitySpec.Handle,
				ActivationKey);

			// 원격(클라이언트 → 서버): 표준 GAS 복제 이벤트 경로.
			// ServerSetReplicatedEvent 가 서버에서 InvokeReplicatedEvent 를 발화시켜
			// 서버 GA 의 WaitInputPress::OnPress 가 호출됩니다.
			// CurrentPredictionKey 인자에도 activation key 를 그대로 넘겨 재사용합니다.
			if (!IsOwnerActorAuthoritative())
			{
				ServerSetReplicatedEvent(
					EAbilityGenericReplicatedEvent::InputPressed,
					AbilitySpec.Handle,
					ActivationKey,
					ActivationKey);
			}
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
			if (AbilitySpec.Ability && AbilitySpec.Ability->bReplicateInputDirectly && !IsOwnerActorAuthoritative())
			{
				ServerSetInputReleased(AbilitySpec.Handle);
			}

			AbilitySpecInputReleased(AbilitySpec);

			const FGameplayAbilityActivationInfo* ActivationInfo = &AbilitySpec.ActivationInfo;
			const TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
			if (AbilityInstances.Num() > 0 && AbilityInstances.Last())
			{
				ActivationInfo = &AbilityInstances.Last()->GetCurrentActivationInfoRef();
			}

			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputReleased,
				AbilitySpec.Handle,
				ActivationInfo->GetActivationPredictionKey());
		}
	}
}

const FBlackoutAbilityInputSyncPayload* UBlackoutAbilitySystemComponent::GetLatestInputSyncPayload(EBlackoutAbilityInputID InputID) const
{
	const int32 RawInputID = static_cast<int32>(InputID);
	return LatestInputSyncPayloadByID.Find(RawInputID);
}

void UBlackoutAbilitySystemComponent::Server_RecordAbilityInputSyncPayload_Implementation(FBlackoutAbilityInputSyncPayload Payload)
{
	// TDD §4.1 v2: 이 RPC 는 timestamp/sequence 메타데이터 부가 채널입니다.
	// 입력 트리거(InputPressed 복제 이벤트) 자체는 표준 GAS 경로(`ServerSetReplicatedEvent`)가 담당하므로,
	// 여기서는 grace clamp 계산에 사용할 페이로드만 기록합니다.
	RecordAbilityInputSyncPayload(Payload, true);
}

uint16 UBlackoutAbilitySystemComponent::GenerateNextInputSequence(EBlackoutAbilityInputID InputID)
{
	const int32 RawInputID = static_cast<int32>(InputID);
	uint16& SequenceId = LocalInputSequenceByID.FindOrAdd(RawInputID);
	++SequenceId;
	if (SequenceId == 0)
	{
		++SequenceId;
	}

	return SequenceId;
}

float UBlackoutAbilitySystemComponent::GetEstimatedServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

FBlackoutAbilityInputSyncPayload UBlackoutAbilitySystemComponent::BuildInputSyncPayload(
	EBlackoutAbilityInputID InputID,
	FGameplayAbilitySpecHandle AbilitySpecHandle,
	uint16 SequenceId,
	float ClientInputTimeSeconds,
	float ClientEstimatedServerTimeSeconds) const
{
	FBlackoutAbilityInputSyncPayload Payload;
	Payload.SequenceId = SequenceId;
	Payload.ClientInputTimeSeconds = ClientInputTimeSeconds;
	Payload.ClientEstimatedServerTimeSeconds = ClientEstimatedServerTimeSeconds;
	Payload.ServerReceivedTimeSeconds = GetEstimatedServerTimeSeconds();
	Payload.InputID = InputID;
	Payload.AbilitySpecHandle = AbilitySpecHandle;
	return Payload;
}

void UBlackoutAbilitySystemComponent::RecordAbilityInputSyncPayload(FBlackoutAbilityInputSyncPayload Payload, bool bValidateSequence)
{
	if (!Payload.IsValid())
	{
		return;
	}

	if (bValidateSequence && !IsInputSequenceNewer(Payload.InputID, Payload.SequenceId))
	{
		BO_LOG_GAS(Verbose,
			"InputSyncPayload rejected: InputID=%d Sequence=%u",
			static_cast<int32>(Payload.InputID),
			Payload.SequenceId);
		return;
	}

	const int32 RawInputID = static_cast<int32>(Payload.InputID);
	Payload.ServerReceivedTimeSeconds = GetEstimatedServerTimeSeconds();
	LatestInputSyncPayloadByID.FindOrAdd(RawInputID) = Payload;

	if (bValidateSequence)
	{
		LastServerInputSequenceByID.FindOrAdd(RawInputID) = Payload.SequenceId;
	}
}

bool UBlackoutAbilitySystemComponent::IsInputSequenceNewer(EBlackoutAbilityInputID InputID, uint16 NewSequenceId) const
{
	if (NewSequenceId == 0)
	{
		return false;
	}

	const int32 RawInputID = static_cast<int32>(InputID);
	const uint16* LastSequenceId = LastServerInputSequenceByID.Find(RawInputID);
	if (!LastSequenceId)
	{
		return true;
	}

	const uint16 Delta = NewSequenceId - *LastSequenceId;
	return Delta != 0 && Delta < 32768;
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

void UBlackoutAbilitySystemComponent::ApplyHealthRegenOverTime(float HealAmountPerTick, float Duration, float TickInterval, FGameplayTag SourceConsumableTag)
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
	ActiveHealthRegenSourceTag = SourceConsumableTag;

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

void UBlackoutAbilitySystemComponent::CancelHealthRegenOverTime()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	if (RemainingHealthRegenTickCount <= 0 && HealthRegenAmountPerTick <= 0.0f)
	{
		return;
	}

	const FGameplayTag CancelledSourceTag = ActiveHealthRegenSourceTag;
	StopHealthRegen();
	if (CancelledSourceTag.IsValid())
	{
		ResetConsumableCooldownForTag(CancelledSourceTag);
		Client_ResetConsumableCooldown(CancelledSourceTag);
	}

	BO_LOG_GAS(Log, "지속 체력 회복 취소: Owner=%s", *GetNameSafe(GetOwner()));
}

void UBlackoutAbilitySystemComponent::Client_ResetConsumableCooldown_Implementation(FGameplayTag ConsumableTag)
{
	ResetConsumableCooldownForTag(ConsumableTag);
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
	
	// 다운/사망 상태로 넘어간 회복 효과는 다음 틱에서 되살리지 않도록 중단합니다.
	if (MaxHealth <= 0.0f || CurrentHealth <= 0.0f || HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed))
	{
		CancelHealthRegenOverTime();
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
	ActiveHealthRegenSourceTag = FGameplayTag();
}

void UBlackoutAbilitySystemComponent::ResetConsumableCooldownForTag(FGameplayTag ConsumableTag)
{
	if (!ConsumableTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const UBOConsumableData* ConsumableData = Cast<UBOConsumableData>(AbilitySpec.SourceObject.Get());
		if (!ConsumableData || !ConsumableData->ConsumableTag.MatchesTagExact(ConsumableTag))
		{
			continue;
		}

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			if (UBlackoutGA_UseConsumable* ConsumableAbility = Cast<UBlackoutGA_UseConsumable>(AbilityInstance))
			{
				ConsumableAbility->ResetConsumableCooldown();
			}
		}
	}
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
