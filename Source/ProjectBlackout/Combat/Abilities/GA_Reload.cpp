#include "Combat/Abilities/GA_Reload.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 장전 애니메이션 몽타주 재생
	if (ReloadMontage)
	{
		// TODO: PlayMontageAndWait 어빌리티 태스크를 통해 몽타주 재생 및 완료 콜백 연결
	}
	else
	{
		OnReloadMontageCompleted();
	}
}

void UGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Reload::OnReloadMontageCompleted()
{
	// 장전 완료 시 데미지 이펙트 (ReloadEffectClass, 내부적으로 ExecCalc_Reload 실행) 적용하여 탄약 수치 갱신
	if (ReloadEffectClass && GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ReloadEffectClass, GetAbilityLevel());
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
