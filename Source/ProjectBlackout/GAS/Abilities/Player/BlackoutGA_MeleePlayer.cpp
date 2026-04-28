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
	bReplicateInputDirectly = true;
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

	if (!MeleeMontage)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: MeleeMontage가 비어 있음");
		HandleMeleeHitNotify();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const FName StartSectionName =
		(ComboSectionNames.Num() > 0 && ComboSectionNames[0] != NAME_None)
			? ComboSectionNames[0]
			: NAME_None;

	bool bMontageStarted = false;
	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (StartSectionName != NAME_None && MeleeMontage->GetSectionIndex(StartSectionName) == INDEX_NONE)
		{
			BO_LOG_GAS(Warning,
				"GA_MeleePlayer failed: 첫 콤보 섹션 %s 이(가) 몽타주 %s 에 없음",
				*StartSectionName.ToString(),
				*GetNameSafe(MeleeMontage));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (PlayerCharacter->HasAuthority())
		{
			PlayerCharacter->Multicast_PlayMeleeMontage(MeleeMontage, StartSectionName, 1.f);
			bMontageStarted = true;
		}
		else if (PlayerCharacter->IsLocallyControlled())
		{
			bMontageStarted = PlayerCharacter->PlayMeleeMontage(MeleeMontage, StartSectionName, 1.f);
		}
		else
		{
			bMontageStarted = PlayerCharacter->PlayMeleeMontage(MeleeMontage, StartSectionName, 1.f);
		}
	}
	else
	{
		UAnimInstance* AnimInstance = GetAvatarAnimInstance();
		if (!AnimInstance)
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer failed: AnimInstance가 비어 있음");
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		const float PlayResult = AnimInstance->Montage_Play(MeleeMontage);
		if (PlayResult > 0.0f && StartSectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(StartSectionName, MeleeMontage);
		}

		bMontageStarted = PlayResult > 0.0f;
	}

	if (!bMontageStarted)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: MeleeMontage 재생 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimInstance* AnimInstance = GetAvatarAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: 재생 후 AnimInstance가 비어 있음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveAnimInstance = AnimInstance;

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
			if (bWasCancelled && PlayerCharacter->HasAuthority())
			{
				PlayerCharacter->Multicast_StopMeleeMontage(MeleeMontage, 0.1f);
			}

			if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
			{
				CombatComponent->EndMeleeHitWindow();
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
		if (CombatComponent->IsMeleeHitWindowActive())
		{
			BO_LOG_GAS(Verbose, "GA_MeleePlayer hit notify ignored: 활성 히트 윈도우가 이미 데미지를 처리 중");
			return;
		}

		CombatComponent->PerformMeleeHit(DamageEffectClass, GetAbilityLevel());
	}
}

void UBlackoutGA_MeleePlayer::HandleMeleeCollisionEnabled()
{
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr)
	{
		CombatComponent->BeginMeleeHitWindow(DamageEffectClass, GetAbilityLevel());
	}
}

void UBlackoutGA_MeleePlayer::HandleMeleeCollisionDisabled()
{
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr)
	{
		CombatComponent->EndMeleeHitWindow();
	}
}

void UBlackoutGA_MeleePlayer::HandleComboWindowOpened()
{
	if (bComboWindowOpen)
	{
		BO_LOG_GAS(Verbose, "GA_MeleePlayer combo window open ignored: 이미 입력 윈도우가 열려 있음");
		return;
	}

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
		if (!JumpToNextComboSection())
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer combo jump failed: CurrentComboIndex=%d", CurrentComboIndex);
		}

		return;
	}

	bComboInputQueued = false;

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
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed to jump combo section: AnimInstance 또는 MeleeMontage가 유효하지 않음");
		return false;
	}

	const int32 NextComboIndex = CurrentComboIndex + 1;
	if (!ComboSectionNames.IsValidIndex(NextComboIndex) || ComboSectionNames[NextComboIndex] == NAME_None)
	{
		bComboInputQueued = false;
		BO_LOG_GAS(Warning,
			"GA_MeleePlayer failed to jump combo section: NextComboIndex=%d 에 유효한 섹션 이름이 없음",
			NextComboIndex);
		return false;
	}

	const FName TargetSectionName = ComboSectionNames[NextComboIndex];
	if (MeleeMontage->GetSectionIndex(TargetSectionName) == INDEX_NONE)
	{
		bComboInputQueued = false;
		BO_LOG_GAS(Warning,
			"GA_MeleePlayer failed to jump combo section: 섹션 %s 이(가) 몽타주 %s 에 없음",
			*TargetSectionName.ToString(),
			*GetNameSafe(MeleeMontage));
		return false;
	}

	if (!ActiveAnimInstance->Montage_IsPlaying(MeleeMontage))
	{
		bComboInputQueued = false;
		BO_LOG_GAS(Warning,
			"GA_MeleePlayer failed to jump combo section: 몽타주가 이미 재생 중이 아님 (TargetSection=%s)",
			*TargetSectionName.ToString());
		return false;
	}

	const FName PreviousSectionName = ActiveAnimInstance->Montage_GetCurrentSection(MeleeMontage);
	bComboInputQueued = false;
	ActiveAnimInstance->Montage_JumpToSection(TargetSectionName, MeleeMontage);

	const FName CurrentSectionName = ActiveAnimInstance->Montage_GetCurrentSection(MeleeMontage);
	if (CurrentSectionName != TargetSectionName)
	{
		BO_LOG_GAS(Warning,
			"GA_MeleePlayer failed to jump combo section: Previous=%s Target=%s Current=%s",
			*PreviousSectionName.ToString(),
			*TargetSectionName.ToString(),
			*CurrentSectionName.ToString());
		return false;
	}

	CurrentComboIndex = NextComboIndex;

	if (ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		if (PlayerCharacter->HasAuthority())
		{
			PlayerCharacter->Multicast_JumpMeleeMontageSection(MeleeMontage, TargetSectionName);
		}
	}

	BO_LOG_GAS(Log,
		"GA_MeleePlayer jumped to combo section: Index=%d Section=%s",
		CurrentComboIndex,
		*TargetSectionName.ToString());

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
