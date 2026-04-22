#include "GAS/Abilities/Player/BlackoutGA_Sprint.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
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
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < MinActivationStamina || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplySprintSpeed(ActorInfo);

	if (UBlackoutCombatComponent* CombatComponent = ActorInfo->AvatarActor->FindComponentByClass<UBlackoutCombatComponent>())
	{
		CombatComponent->StopAim();
	}

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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBlackoutGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
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

	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	const float BaseMovementSpeed = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMovementSpeedAttribute())
		: CachedWalkSpeed;

	const float TargetBaseSpeed = BaseMovementSpeed > 0.0f ? BaseMovementSpeed : CachedWalkSpeed;
	MovementComponent->MaxWalkSpeed = TargetBaseSpeed * SprintSpeedMultiplier;
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

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < StaminaDrainPerTick)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -StaminaDrainPerTick);
	return true;
}
