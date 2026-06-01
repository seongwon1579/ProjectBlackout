#include "GAS/Abilities/Player/BlackoutGA_UseRelic.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_UseRelic::UBlackoutGA_UseRelic()
{
	InputID = EBlackoutAbilityInputID::UseRelic;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(BlackoutGameplayTags::Ability_Player_UseRelic);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_UseRelic);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Attacking);

	RelicUseCueTag = BlackoutGameplayTags::GameplayCue_Relic_Use;
}

bool UBlackoutGA_UseRelic::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags,
	                               OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!AbilitySystemComponent || !PlayerCharacter)
	{
		return false;
	}

	const float CurrentRelicCharges = AbilitySystemComponent->
		GetNumericAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute());
	if (CurrentRelicCharges < static_cast<float>(RelicChargeCost))
	{
		return false;
	}

	const float CurrentHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	return MaxHealth > 0.0f && CurrentHealth < MaxHealth;
}

void UBlackoutGA_UseRelic::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		                                            ? Cast<
			                                            ABlackoutPlayerCharacter>(
			                                            ActorInfo->AvatarActor.
			                                            Get())
		                                            : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent =
		GetAbilitySystemComponentFromActorInfo();
	if (!PlayerCharacter || !AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "유물 사용 실패: PlayerCharacter 또는 ASC가 유효하지 않습니다.");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentRelicCharges = AbilitySystemComponent->
		GetNumericAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute());
	if (CurrentRelicCharges < static_cast<float>(RelicChargeCost))
	{
		BO_LOG_GAS(Warning,
		           "유물 사용 실패: 충전 횟수가 부족합니다. Player=%s Current=%.0f Need=%d",
		           *GetNameSafe(PlayerCharacter),
		           CurrentRelicCharges,
		           RelicChargeCost);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.0f || CurrentHealth >= MaxHealth)
	{
		BO_LOG_GAS(Warning,
		           "유물 사용 실패: 회복할 체력이 없습니다. Player=%s Health=%.1f Max=%.1f",
		           *GetNameSafe(PlayerCharacter),
		           CurrentHealth,
		           MaxHealth);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "유물 사용 실패: CommitAbility 실패. Player=%s",
		           *GetNameSafe(PlayerCharacter));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bRelicApplied = false;

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->
		GetCombatComponent())
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		// CombatComponent->StopAim();
	}

	ApplySlowMovementSpeed(ActorInfo);
	BeginWeaponHolsterOverride(ActorInfo);

	if (RelicMontage)
	{
		const float MontageDuration = RelicMontage->GetPlayLength();
		if (MontageDuration > 0.0f)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				PlayerCharacter->PlayAnimMontage(RelicMontage, 1.0f);
			}

			if (PlayerCharacter->HasAuthority())
			{
				PlayerCharacter->Multicast_PlayConsumableMontage(
					RelicMontage, 1.0f);

				UAbilityTask_WaitGameplayEvent* WaitEventTask =
					UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
						this, BlackoutGameplayTags::Event_Montage_RelicApply);
				if (WaitEventTask)
				{
					WaitEventTask->EventReceived.AddDynamic(
						this, &UBlackoutGA_UseRelic::OnRelicApplyEventReceived);
					WaitEventTask->ReadyForActivation();
				}

				if (UWorld* World = PlayerCharacter->GetWorld())
				{
					World->GetTimerManager().SetTimer(
						RelicMontageTimerHandle,
						this,
						&UBlackoutGA_UseRelic::OnRelicMontageFinished,
						MontageDuration,
						false);
				}

				BO_LOG_GAS(Log, "유물 몽타주 시작: Player=%s Duration=%.2f",
				           *GetNameSafe(PlayerCharacter),
				           MontageDuration);
			}

			return;
		}
	}

	if (PlayerCharacter->HasAuthority())
	{
		ApplyRelicEffect();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBlackoutGA_UseRelic::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo*
                                      ActorInfo,
                                      const FGameplayAbilityActivationInfo
                                      ActivationInfo, bool bReplicateEndAbility,
                                      bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(RelicMontageTimerHandle);
		}

		if (bWasCancelled && RelicMontage)
		{
			if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<
				ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
			{
				PlayerCharacter->Multicast_StopConsumableMontage(RelicMontage);
			}
		}

		RestoreMovementSpeed(ActorInfo);
		EndWeaponHolsterOverride(ActorInfo);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
	                  bWasCancelled);
}

void UBlackoutGA_UseRelic::ApplyRelicEffect()
{
	if (bRelicApplied)
	{
		return;
	}

	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !
		CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		GetAbilitySystemComponentFromActorInfo();
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo
		? Cast<ABlackoutPlayerCharacter>(
			CurrentActorInfo->AvatarActor.Get())
		: nullptr;
	if (!AbilitySystemComponent || !PlayerCharacter)
	{
		BO_LOG_GAS(Warning, "유물 적용 실패: ASC 또는 PlayerCharacter가 유효하지 않습니다.");
		return;
	}

	const float CurrentRelicCharges = AbilitySystemComponent->
		GetNumericAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute());
	if (CurrentRelicCharges < static_cast<float>(RelicChargeCost))
	{
		BO_LOG_GAS(Warning,
		           "유물 적용 실패: 적용 시점에 충전 횟수가 부족합니다. Player=%s Current=%.0f Need=%d",
		           *GetNameSafe(PlayerCharacter),
		           CurrentRelicCharges,
		           RelicChargeCost);
		return;
	}

	const float CurrentHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.0f || CurrentHealth >= MaxHealth)
	{
		BO_LOG_GAS(Warning,
		           "유물 적용 실패: 회복할 체력이 없습니다. Player=%s Health=%.1f Max=%.1f",
		           *GetNameSafe(PlayerCharacter),
		           CurrentHealth,
		           MaxHealth);
		return;
	}

	bRelicApplied = true;

	const float HealingEffectivenessAttribute = AbilitySystemComponent->
		GetNumericAttribute(
			UBlackoutPlayerAttributeSet::GetHealingEffectivenessAttribute());
	const float HealingEffectiveness = HealingEffectivenessAttribute > 0.0f
		                                   ? HealingEffectivenessAttribute
		                                   : 1.0f;
	const float EffectiveHealAmount = FMath::Min(
		RelicHealAmount * HealingEffectiveness, MaxHealth - CurrentHealth);

	const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<
		UBlackoutAbilitySystemComponent>(AbilitySystemComponent);
	const bool bSkipCost = BlackoutASC && BlackoutASC->
		ShouldSkipCostInShelter();

	if (!bSkipCost)
	{
		AbilitySystemComponent->ApplyModToAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute(),
			EGameplayModOp::Additive,
			-static_cast<float>(RelicChargeCost));
	}

	if (EffectiveHealAmount > 0.0f)
	{
		AbilitySystemComponent->ApplyModToAttribute(
			UBlackoutBaseAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			EffectiveHealAmount);
	}

	ApplyRelicHealEffect();
	ExecuteRelicUseCue();

	const int32 RemainingRelicCharges = FMath::RoundToInt(
		AbilitySystemComponent->GetNumericAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute()));
	ReceiveRelicUsed(EffectiveHealAmount, RemainingRelicCharges);

	BO_LOG_GAS(Log, "유물 사용 완료: Player=%s Heal=%.1f Remaining=%d",
	           *GetNameSafe(PlayerCharacter),
	           EffectiveHealAmount,
	           RemainingRelicCharges);
}

void UBlackoutGA_UseRelic::ExecuteRelicUseCue()
{
	if (!RelicUseCueTag.IsValid())
	{
		return;
	}

	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !
		CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		GetAbilitySystemComponentFromActorInfo();
	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(
		CurrentActorInfo->AvatarActor.Get());
	USkeletalMeshComponent* CharacterMesh = PlayerCharacter
		                                        ? PlayerCharacter->GetMesh()
		                                        : nullptr;
	if (!AbilitySystemComponent || !PlayerCharacter || !CharacterMesh)
	{
		BO_LOG_GAS(Warning,
		           "유물 GCN 실행 실패: ASC, PlayerCharacter 또는 Mesh가 유효하지 않습니다. ASC=%s Player=%s Mesh=%s",
		           *GetNameSafe(AbilitySystemComponent),
		           *GetNameSafe(PlayerCharacter),
		           *GetNameSafe(CharacterMesh));
		return;
	}

	if (RelicUseCueSocketName.IsNone() || !CharacterMesh->DoesSocketExist(
		RelicUseCueSocketName))
	{
		BO_LOG_GAS(Warning,
		           "유물 GCN 실행 실패: 오른손 소켓이 유효하지 않습니다. Player=%s Socket=%s",
		           *GetNameSafe(PlayerCharacter),
		           *RelicUseCueSocketName.ToString());
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = CharacterMesh->GetSocketLocation(
		RelicUseCueSocketName);
	CueParameters.Normal = CharacterMesh->GetSocketRotation(
		RelicUseCueSocketName).Vector();
	CueParameters.Instigator = PlayerCharacter->GetInstigator();
	CueParameters.EffectCauser = PlayerCharacter;
	CueParameters.SourceObject = PlayerCharacter;
	CueParameters.TargetAttachComponent = CharacterMesh;

	AbilitySystemComponent->ExecuteGameplayCue(RelicUseCueTag, CueParameters);
}

void UBlackoutGA_UseRelic::OnRelicApplyEventReceived(FGameplayEventData Payload)
{
	ApplyRelicEffect();
}

void UBlackoutGA_UseRelic::OnRelicMontageFinished()
{
	ApplyRelicEffect();
	RestoreMovementSpeed(CurrentActorInfo);
	EndWeaponHolsterOverride(CurrentActorInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true,
	           !bRelicApplied);
}

void UBlackoutGA_UseRelic::ApplyRelicHealEffect()
{
	if (!RelicHealEffect)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		RelicHealEffect, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		BO_LOG_GAS(Warning, "유물 보조 GE 생성 실패: Effect=%s",
		           *GetNameSafe(RelicHealEffect.Get()));
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
		*SpecHandle.Data.Get());
}

void UBlackoutGA_UseRelic::ApplySlowMovementSpeed(
	const FGameplayAbilityActorInfo* ActorInfo)
{
	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;
	UCharacterMovementComponent* MovementComponent = PlayerCharacter
		? PlayerCharacter->GetCharacterMovement()
		: nullptr;
	if (!MovementComponent)
	{
		return;
	}

	CachedWalkSpeed = MovementComponent->MaxWalkSpeed;
	// 에디터에서 배율이 0으로 남아 있어도 이동 가능하도록 안전한 기본값을 보장합니다.
	const float EffectiveRelicSpeedMultiplier = RelicSpeedMultiplier > 0.0f
		                                            ? RelicSpeedMultiplier
		                                            : 0.35f;
	MovementComponent->MaxWalkSpeed = CachedWalkSpeed *
		EffectiveRelicSpeedMultiplier;
}

void UBlackoutGA_UseRelic::RestoreMovementSpeed(
	const FGameplayAbilityActorInfo* ActorInfo)
{
	if (CachedWalkSpeed <= 0.0f)
	{
		return;
	}

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter
		? PlayerCharacter->GetCharacterMovement()
		: nullptr)
	{
		MovementComponent->MaxWalkSpeed = CachedWalkSpeed;
	}

	CachedWalkSpeed = 0.0f;
}

void UBlackoutGA_UseRelic::BeginWeaponHolsterOverride(
	const FGameplayAbilityActorInfo* ActorInfo)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		                                            ? Cast<
			                                            ABlackoutPlayerCharacter>(
			                                            ActorInfo->AvatarActor.
			                                            Get())
		                                            : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter
		                                            ? PlayerCharacter->
		                                            GetCombatComponent()
		                                            : nullptr;
	if (!CombatComponent)
	{
		BO_LOG_GAS(Warning,
		           "유물 사용 무기 홀스터 실패: CombatComponent가 유효하지 않습니다. Player=%s",
		           *GetNameSafe(PlayerCharacter));
		return;
	}

	CombatComponent->BeginEquippedWeaponHolsterOverride();
}

void UBlackoutGA_UseRelic::EndWeaponHolsterOverride(
	const FGameplayAbilityActorInfo* ActorInfo)
{
	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo
		                                            ? Cast<
			                                            ABlackoutPlayerCharacter>(
			                                            ActorInfo->AvatarActor.
			                                            Get())
		                                            : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter
		                                            ? PlayerCharacter->
		                                            GetCombatComponent()
		                                            : nullptr;
	if (!CombatComponent)
	{
		return;
	}

	CombatComponent->EndEquippedWeaponHolsterOverride();
}
