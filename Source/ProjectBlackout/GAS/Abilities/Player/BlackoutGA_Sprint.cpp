#include "GAS/Abilities/Player/BlackoutGA_Sprint.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Sprint::UBlackoutGA_Sprint()
{
	InputID = EBlackoutAbilityInputID::Sprint;
	bReplicateInputDirectly = true;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Sprint activate requested");

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_Sprint failed: ActorInfo 또는 AvatarActor가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_Sprint failed: AbilitySystemComponent가 없음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < MinActivationStamina || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Sprint failed: 스태미나 부족 또는 CommitAbility 실패 (CurrentStamina=%.2f)", CurrentStamina);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_Sprint activated: Character=%s", *GetNameSafe(ActorInfo->AvatarActor.Get()));

	if (UBlackoutCombatComponent* CombatComponent = ActorInfo->AvatarActor->FindComponentByClass<UBlackoutCombatComponent>())
	{
		CombatComponent->StopAim();
	}

	ApplySprintSpeed(ActorInfo);

	if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
	{
		World->GetTimerManager().SetTimer(SprintDrainTimerHandle, this, &UBlackoutGA_Sprint::HandleSprintTick, StaminaDrainInterval, true, StaminaDrainInterval);
	}
}

void UBlackoutGA_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (IsActive())
	{
		BO_LOG_GAS(Log, "GA_Sprint input released");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBlackoutGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Sprint ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(SprintDrainTimerHandle);
		}

		RestoreWalkSpeed(ActorInfo);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Sprint::HandleSprintTick()
{
	if (!IsActive() || !ConsumeSprintStamina())
	{
		BO_LOG_GAS(Log, "GA_Sprint finishing: 비활성 상태이거나 스태미나가 부족함");
		K2_EndAbility();
	}
}

void UBlackoutGA_Sprint::ApplySprintSpeed(const FGameplayAbilityActorInfo* ActorInfo)
{
	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	if (!MovementComponent)
	{
		return;
	}

	CachedWalkSpeed = MovementComponent->MaxWalkSpeed;
	MovementComponent->MaxWalkSpeed = CachedWalkSpeed * SprintSpeedMultiplier;
}

void UBlackoutGA_Sprint::RestoreWalkSpeed(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr)
	{
		MovementComponent->MaxWalkSpeed = CachedWalkSpeed > 0.0f ? CachedWalkSpeed : MovementComponent->MaxWalkSpeed;
	}
}

bool UBlackoutGA_Sprint::ConsumeSprintStamina() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	const UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent);
	const float StaminaCostMultiplier = BlackoutAbilitySystemComponent ? BlackoutAbilitySystemComponent->GetStaminaCostMultiplier() : 1.0f;
	const float ModifiedStaminaDrain = StaminaDrainPerTick * StaminaCostMultiplier;

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < ModifiedStaminaDrain)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -ModifiedStaminaDrain);

	if (UBlackoutAbilitySystemComponent* MutableBlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent))
	{
		MutableBlackoutAbilitySystemComponent->NotifyStaminaSpent();
	}

	return true;
}
