#include "Combat/Abilities/GA_FireWeapon.h"
#include "AbilitySystemComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GameFramework/Character.h"

UGA_FireWeapon::UGA_FireWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// TODO: LobbyTag.InfiniteAmmo 분기로 탄약 소모 체크 생략 로직 추가 (TDD §7.1)
}

void UGA_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 탄약 소모 확인 및 적용
	if (!ApplyAmmoCost())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 사격 애니메이션 몽타주 재생
	PlayFireMontage();

	// 3. 트레이스 수행 또는 발사체 스폰 (무기 컴포넌트 정보 기반)
	// TODO: ABOFirearm::Fire 호출 및 피격 결과에 따른 데미지 이펙트 적용

	// 4. 사격 GameplayCue 트리거
	// TODO: GCN_Weapon_Fire 일회성 이벤트 트리거

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_FireWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FHitResult UGA_FireWeapon::PerformTrace(const FVector& Start, const FVector& End)
{
	FHitResult HitResult;
	// TODO: 무기 유효 사거리 및 카메라-총구 간 크로스헤어 보정을 포함한 라인 트레이스 구현
	return HitResult;
}

FGameplayEffectSpecHandle UGA_FireWeapon::BuildDamageSpec(const FHitResult& HitResult)
{
	FGameplayEffectSpecHandle SpecHandle;
	if (DamageEffectClass && GetAbilitySystemComponentFromActorInfo())
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		// TODO: HitResult에서 Hitbox 컴포넌트의 부위 태그를 확인하고 SetByCaller로 데미지 배율(Body.WeakSpot 등) 주입
	}
	return SpecHandle;
}

bool UGA_FireWeapon::ApplyAmmoCost()
{
	// TODO: 장착된 무기(주무기/보조무기)의 태그를 식별하여 해당하는 ClipAmmo를 1 차감
	return true; // 임시 성공 처리
}

void UGA_FireWeapon::PlayFireMontage()
{
	// TODO: 캐릭터 사격 몽타주 재생 (PlayMontageAndWait 활용)
}
