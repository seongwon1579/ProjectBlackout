#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Dodge::UBlackoutGA_Dodge()
{
	InputID = EBlackoutAbilityInputID::Dodge;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Invulnerable);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter || !ConsumeStamina())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(ActorInfo, bIsBackstep);
	if (DodgeDirection.IsNearlyZero())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	const float LaunchStrength = bIsBackstep ? BackstepStrength : DodgeStrength;
	PlayerCharacter->LaunchCharacter(DodgeDirection * LaunchStrength + FVector::UpVector * UpwardImpulse, true, true);

	if (DodgeMontage)
	{
		if (UAnimInstance* AnimInstance = PlayerCharacter->GetMesh() ? PlayerCharacter->GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(DodgeMontage);
		}
	}

	if (UWorld* World = PlayerCharacter->GetWorld())
	{
		World->GetTimerManager().SetTimer(DodgeEndTimerHandle, this, &UBlackoutGA_Dodge::OnDodgeFinished, DodgeDuration, false);
	}
}

void UBlackoutGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DodgeEndTimerHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Dodge::OnDodgeFinished()
{
	if (!IsActive())
	{
		return;
	}

	K2_EndAbility();
}

bool UBlackoutGA_Dodge::ConsumeStamina() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < StaminaCost)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -StaminaCost);
	return true;
}

FVector UBlackoutGA_Dodge::CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep) const
{
	bOutIsBackstep = true;

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector LastInputDirection = PlayerCharacter->GetLastMovementInputVector().GetSafeNormal2D();
	if (!LastInputDirection.IsNearlyZero())
	{
		bOutIsBackstep = false;
		return LastInputDirection;
	}

	return -PlayerCharacter->GetActorForwardVector().GetSafeNormal2D();
}
