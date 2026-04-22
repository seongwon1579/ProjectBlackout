#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

#include "Animation/AnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "Core/BlackoutLog.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "TimerManager.h"

UBlackoutGA_MeleePlayer::UBlackoutGA_MeleePlayer()
{
	InputID = EBlackoutAbilityInputID::Melee;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
}

void UBlackoutGA_MeleePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer activate requested");

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer activated: Character=%s", *GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr));

	// 1. 근접 공격 애니메이션 몽타주 재생 및 콤보 윈도우(입력 대기) 활성화
	float MontageDuration = 0.0f;
	if (MeleeMontage)
	{
		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
			{
				MontageDuration = AnimInstance->Montage_Play(MeleeMontage);
			}
		}
	}

	if (UWorld* World = ActorInfo && ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(MeleeHitTimerHandle, this, &UBlackoutGA_MeleePlayer::OnMeleeHitNotify, FMath::Max(0.0f, MeleeHitDelay), false);

		const float FinishDelay = MontageDuration > 0.0f ? MontageDuration : FMath::Max(0.2f, MeleeHitDelay + 0.05f);
		World->GetTimerManager().SetTimer(MeleeFinishTimerHandle, this, &UBlackoutGA_MeleePlayer::OnMeleeAttackFinished, FinishDelay, false);
		return;
	}

	OnMeleeHitNotify();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UBlackoutGA_MeleePlayer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(MeleeHitTimerHandle);
			World->GetTimerManager().ClearTimer(MeleeFinishTimerHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_MeleePlayer::OnMeleeHitNotify()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer hit notify");

	// 몽타주 내의 AnimNotify에서 어빌리티 태스크를 통해 호출됨
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr)
	{
		CombatComponent->PerformMeleeHit();
	}
}

void UBlackoutGA_MeleePlayer::OnMeleeAttackFinished()
{
	if (!IsActive())
	{
		return;
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer finished by timer");
	K2_EndAbility();
}
