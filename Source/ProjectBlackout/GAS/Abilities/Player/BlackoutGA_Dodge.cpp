#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "Core/BlackoutLog.h"
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
	BO_LOG_GAS(Log, "GA_Dodge activate requested");

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: ActorInfo 또는 AvatarActor가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter || !ConsumeStamina())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: PlayerCharacter가 없거나 스태미나가 부족함");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(ActorInfo, bIsBackstep);
	if (DodgeDirection.IsNearlyZero())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 방향 계산 결과가 0 벡터");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_Dodge activated: Character=%s, Backstep=%s", *GetNameSafe(PlayerCharacter), bIsBackstep ? TEXT("true") : TEXT("false"));

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
	BO_LOG_GAS(Log, "GA_Dodge ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

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

	BO_LOG_GAS(Log, "GA_Dodge finished by timer");
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

	if (UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent))
	{
		BlackoutAbilitySystemComponent->NotifyStaminaSpent();
	}

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
