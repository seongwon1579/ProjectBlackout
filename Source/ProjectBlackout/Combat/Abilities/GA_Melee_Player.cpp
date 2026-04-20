#include "Combat/Abilities/GA_Melee_Player.h"
#include "Combat/Weapons/BOMeleeWeapon.h"

UGA_Melee_Player::UGA_Melee_Player()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Melee_Player::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 근접 공격 애니메이션 몽타주 재생 및 콤보 윈도우(입력 대기) 활성화
	if (MeleeMontage)
	{
		// TODO: PlayMontageAndWait 어빌리티 태스크를 통해 몽타주 실행 및 콤보 연결
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Melee_Player::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Melee_Player::OnMeleeHitNotify()
{
	// 몽타주 내의 AnimNotify에서 어빌리티 태스크를 통해 호출됨
	// TODO: 장착된 근접 무기의 ABOMeleeWeapon::PerformSweep 호출 및 피격 결과에 따른 GE_Damage 적용
}
