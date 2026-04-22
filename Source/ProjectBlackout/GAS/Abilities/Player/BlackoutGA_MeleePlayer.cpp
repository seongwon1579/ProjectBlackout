#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

#include "Animation/AnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "GameFramework/Character.h"
#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutGA_MeleePlayer::UBlackoutGA_MeleePlayer()
{
	InputID = EBlackoutAbilityInputID::Melee;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
}

void UBlackoutGA_MeleePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 근접 공격 애니메이션 몽타주 재생 및 콤보 윈도우(입력 대기) 활성화
	if (MeleeMontage)
	{
		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInstance->Montage_Play(MeleeMontage);
			}
		}
	}

	OnMeleeHitNotify();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UBlackoutGA_MeleePlayer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_MeleePlayer::OnMeleeHitNotify()
{
	// 몽타주 내의 AnimNotify에서 어빌리티 태스크를 통해 호출됨
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr)
	{
		CombatComponent->PerformMeleeHit();
	}
}
