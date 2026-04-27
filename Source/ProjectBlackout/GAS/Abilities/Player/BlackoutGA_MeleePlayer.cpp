#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/BlackoutLog.h"
#include "GameFramework/Character.h"
#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutGA_MeleePlayer::UBlackoutGA_MeleePlayer()
{
	InputID = EBlackoutAbilityInputID::Melee;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Aiming);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_MeleePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer activate requested");

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: ActorInfo 또는 AvatarActor가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer activated: Character=%s", *GetNameSafe(ActorInfo->AvatarActor.Get()));

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
		{
			CombatComponent->BeginMeleeWeaponAttachmentOverride();
			CombatComponent->StopAim();
		}
	}

	ResetComboState();
	CurrentComboIndex = 0;

	UAnimInstance* AnimInstance = GetAvatarAnimInstance();
	if (!AnimInstance || !MeleeMontage)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: AnimInstance 또는 MeleeMontage가 비어 있음");
		HandleMeleeHitNotify();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const float PlayResult = AnimInstance->Montage_Play(MeleeMontage);
	if (PlayResult <= 0.0f)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: MeleeMontage 재생 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveAnimInstance = AnimInstance;

	if (ComboSectionNames.Num() > 0 && ComboSectionNames[0] != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(ComboSectionNames[0], MeleeMontage);
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UBlackoutGA_MeleePlayer::OnMeleeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MeleeMontage);
}

void UBlackoutGA_MeleePlayer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (UAnimInstance* AnimInstance = ActiveAnimInstance.Get())
	{
		if (bWasCancelled && MeleeMontage && AnimInstance->Montage_IsPlaying(MeleeMontage))
		{
			FOnMontageEnded EmptyMontageEndedDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyMontageEndedDelegate, MeleeMontage);
			AnimInstance->Montage_Stop(0.1f, MeleeMontage);
		}
	}

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
			{
				CombatComponent->EndMeleeWeaponAttachmentOverride();
			}
		}
	}

	ResetComboState();
	ActiveAnimInstance = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_MeleePlayer::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (!bComboWindowOpen)
	{
		return;
	}

	if (!ComboSectionNames.IsValidIndex(CurrentComboIndex + 1))
	{
		return;
	}

	bComboInputQueued = true;
	BO_LOG_GAS(Log, "GA_MeleePlayer combo input queued: CurrentComboIndex=%d", CurrentComboIndex);
}

UBlackoutGA_MeleePlayer* UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(const AActor* OwnerActor)
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
			if (UBlackoutGA_MeleePlayer* MeleeAbility = Cast<UBlackoutGA_MeleePlayer>(AbilityInstance))
			{
				return MeleeAbility;
			}
		}
	}

	return nullptr;
}

void UBlackoutGA_MeleePlayer::HandleMeleeHitNotify()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer hit notify");

	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr)
	{
		CombatComponent->PerformMeleeHit();
	}
}

void UBlackoutGA_MeleePlayer::HandleComboWindowOpened()
{
	bComboWindowOpen = true;
	bComboInputQueued = false;

	BO_LOG_GAS(Log, "GA_MeleePlayer combo window opened: CurrentComboIndex=%d", CurrentComboIndex);
}

void UBlackoutGA_MeleePlayer::HandleComboWindowClosed()
{
	if (!bComboWindowOpen)
	{
		return;
	}

	bComboWindowOpen = false;

	if (bComboInputQueued)
	{
		JumpToNextComboSection();
		return;
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer combo window closed without queued input");
}

void UBlackoutGA_MeleePlayer::OnMeleeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsActive() || Montage != MeleeMontage)
	{
		return;
	}

	BO_LOG_GAS(Log,
		"GA_MeleePlayer montage ended: Interrupted=%s",
		bInterrupted ? TEXT("true") : TEXT("false"));

	K2_EndAbility();
}

bool UBlackoutGA_MeleePlayer::JumpToNextComboSection()
{
	if (!ActiveAnimInstance || !MeleeMontage)
	{
		return false;
	}

	const int32 NextComboIndex = CurrentComboIndex + 1;
	if (!ComboSectionNames.IsValidIndex(NextComboIndex) || ComboSectionNames[NextComboIndex] == NAME_None)
	{
		bComboInputQueued = false;
		return false;
	}

	CurrentComboIndex = NextComboIndex;
	bComboInputQueued = false;
	ActiveAnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboIndex], MeleeMontage);

	BO_LOG_GAS(Log,
		"GA_MeleePlayer jumped to combo section: Index=%d Section=%s",
		CurrentComboIndex,
		*ComboSectionNames[CurrentComboIndex].ToString());

	return true;
}

void UBlackoutGA_MeleePlayer::ResetComboState()
{
	CurrentComboIndex = INDEX_NONE;
	bComboWindowOpen = false;
	bComboInputQueued = false;
}

UAnimInstance* UBlackoutGA_MeleePlayer::GetAvatarAnimInstance() const
{
	const ACharacter* Character = CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	USkeletalMeshComponent* MeshComponent = Character ? Character->GetMesh() : nullptr;
	return MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
}
