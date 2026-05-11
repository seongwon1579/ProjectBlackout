#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Animation/AnimMontage.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Dodge::UBlackoutGA_Dodge()
{
	InputID = EBlackoutAbilityInputID::Dodge;

	// i-frame 을 위한 임시 제거 
	//ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Invulnerable);

	
	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Locked);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_Dodge::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (!IsActive() || !bChainWindowOpen)
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return;
	}

	if (!ConsumeStamina())
	{
		BO_LOG_GAS(Log, "GA_Dodge follow-up skipped: 스태미나 부족");
		return;
	}

	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(ActorInfo, bIsBackstep, true);
	if (DodgeDirection.IsNearlyZero())
	{
		BO_LOG_GAS(Warning, "GA_Dodge follow-up failed: 회피 방향 계산 결과가 0 벡터");
		return;
	}

	bChainWindowOpen = false;
	bIsBackstep = false;

	if (!StartDodgeInternal(PlayerCharacter, DodgeDirection, bIsBackstep, true))
	{
		BO_LOG_GAS(Warning, "GA_Dodge follow-up failed: 회피 재시작에 실패함");
		return;
	}

	BO_LOG_GAS(Log, "GA_Dodge follow-up started immediately");
}

UBlackoutGA_Dodge* UBlackoutGA_Dodge::GetActiveDodgeAbilityFromActor(const AActor* OwnerActor)
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
	const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			if (UBlackoutGA_Dodge* DodgeAbility = Cast<UBlackoutGA_Dodge>(AbilityInstance))
			{
				return DodgeAbility;
			}
		}
	}

	return nullptr;
}

void UBlackoutGA_Dodge::HandleChainWindowOpened()
{
	if (!IsActive())
	{
		return;
	}

	bChainWindowOpen = true;
	BO_LOG_GAS(Log, "GA_Dodge chain window opened");
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

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(PlayerCharacter))
	{
		BO_LOG_GAS(Log, "GA_Dodge cancelling active melee before dodge");
		MeleeAbility->K2_CancelAbility();
	}

	// TODO : 백스텝 임시 항상 FALSE  , 추후 방향입력 X DODGE 실행시 백스텝 추가 예정 
	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(ActorInfo, bIsBackstep);
	if (DodgeDirection.IsNearlyZero())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 방향 계산 결과가 0 벡터");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsBackstep = false; // 현재는 forward roll만 사용

	if (!StartDodgeInternal(PlayerCharacter, DodgeDirection, bIsBackstep, false))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 시작에 실패함");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UBlackoutGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Dodge ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->SetPendingDodgeInput(FVector2D::ZeroVector);
		}

		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DodgeEndTimerHandle);
		}
	}

	bChainWindowOpen = false;

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

bool UBlackoutGA_Dodge::StartDodgeInternal(ABlackoutPlayerCharacter* PlayerCharacter, const FVector& DodgeDirection, bool bIsBackstep, bool bRestartMontage)
{
	if (!PlayerCharacter)
	{
		return false;
	}

	bChainWindowOpen = false;

	const FRotator TargetRotation(0.f, DodgeDirection.Rotation().Yaw, 0.f);
	PlayerCharacter->SetActorRotation(TargetRotation);

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		CombatComponent->StopAim();
	}

	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (DodgeMontage)
	{
		if (PlayerCharacter->IsLocallyControlled())
		{
			PlayerCharacter->PlayDodgeMontage(DodgeMontage, 1.f, bRestartMontage);
		}

		if (PlayerCharacter->HasAuthority())
		{
			PlayerCharacter->Multicast_PlayDodgeMontage(DodgeMontage, 1.f, bRestartMontage);
		}
	}
	else
	{
		BO_LOG_GAS(Warning, "GA_Dodge: DodgeMontage가 설정되지 않아 몽타주를 재생하지 못함");
	}

	const float LaunchStrength = bIsBackstep ? BackstepStrength : DodgeStrength;
	PlayerCharacter->LaunchCharacter(DodgeDirection * LaunchStrength + FVector::UpVector * UpwardImpulse, true, true);

	const float DodgeEndDelay =
		(DodgeMontage && DodgeMontage->GetPlayLength() > 0.f)
			? DodgeMontage->GetPlayLength()
			: DodgeDuration;

	if (UWorld* World = PlayerCharacter->GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeEndTimerHandle);
		World->GetTimerManager().SetTimer(
			DodgeEndTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnDodgeFinished,
			DodgeEndDelay,
			false);
	}

	return true;
}

bool UBlackoutGA_Dodge::ConsumeStamina() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent);
	const float StaminaCostMultiplier = BlackoutASC ? BlackoutASC->GetStaminaCostMultiplier() : 1.0f;
	const float ModifiedStaminaCost = StaminaCost * StaminaCostMultiplier;
	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < ModifiedStaminaCost)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -ModifiedStaminaCost);

	if (UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent))
	{
		BlackoutAbilitySystemComponent->NotifyStaminaSpent();
	}

	return true;
}

FVector UBlackoutGA_Dodge::CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep, bool bPreferControlForwardWhenNoInput) const
{
	bOutIsBackstep = false;

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector2D PendingDodgeInput = PlayerCharacter->GetPendingDodgeInput();
	const FVector2D CachedMoveInput = PlayerCharacter->GetCachedMoveInput();
	const FVector2D InputToUse = !PendingDodgeInput.IsNearlyZero() ? PendingDodgeInput : CachedMoveInput;

	BO_LOG_GAS(Log,
		"DodgeInput: Pending=(%.2f, %.2f), Cached=(%.2f, %.2f)",
		PendingDodgeInput.X, PendingDodgeInput.Y,
		CachedMoveInput.X, CachedMoveInput.Y);

	if (!InputToUse.IsNearlyZero())
	{
		const FRotator ControlRotation = PlayerCharacter->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector ControlForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector ControlRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector WorldInputDirection =
			(ControlForward * InputToUse.Y + ControlRight * InputToUse.X).GetSafeNormal2D();

		if (!WorldInputDirection.IsNearlyZero())
		{
			return WorldInputDirection;
		}
	}

	if (bPreferControlForwardWhenNoInput)
	{
		const FRotator ControlRotation = PlayerCharacter->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	}

	// 서버에서 입력 전송 타이밍이 어긋나도 최소한 실제 이동 방향은 따라가도록 보조
	const FVector LastMovementDirection = PlayerCharacter->GetLastMovementInputVector().GetSafeNormal2D();
	if (!LastMovementDirection.IsNearlyZero())
	{
		return LastMovementDirection;
	}

	const FVector VelocityDirection = PlayerCharacter->GetVelocity().GetSafeNormal2D();
	if (!VelocityDirection.IsNearlyZero())
	{
		return VelocityDirection;
	}

	return PlayerCharacter->GetActorForwardVector().GetSafeNormal2D();
	
}
