// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Wraith_FireTwinArrows.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UGA_Wraith_FireTwinArrows::UGA_Wraith_FireTwinArrows()
{
	AbilityTags.AddTag(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows);
}

void UGA_Wraith_FireTwinArrows::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 활시위 당김 Montage 재생 — 시각 단서. 발사 타이밍은 기존 WaitDelay 유지 (AnimNotify 정렬은 다음 단계)
	if (BowshotMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BowshotMontage);
		if (MontageTask)
		{
			MontageTask->ReadyForActivation();
		}
	}

	// 조준 선딜 (AimDelay) 후 첫 발사 — Montage 의 활시위 당김 모션과 시각 동기화
	UAbilityTask_WaitDelay* AimTask = UAbilityTask_WaitDelay::WaitDelay(this, AimDelay);
	if (!AimTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	AimTask->OnFinish.AddDynamic(this, &UGA_Wraith_FireTwinArrows::OnAimDelayFinished);
	AimTask->ReadyForActivation();
}

void UGA_Wraith_FireTwinArrows::OnAimDelayFinished()
{
	// 첫 발사
	FireOneArrow();

	// 인터벌 후 두 번째 발사 + 종료
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, SecondShotDelay);
	if (!DelayTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	DelayTask->OnFinish.AddDynamic(this, &UGA_Wraith_FireTwinArrows::OnSecondShotDelayFinished);
	DelayTask->ReadyForActivation();
}

void UGA_Wraith_FireTwinArrows::OnSecondShotDelayFinished()
{
	FireOneArrow();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Wraith_FireTwinArrows::FireOneArrow()
{
	if (!ArrowProjectileClass) { return; }

	APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!Avatar) { return; }

	UWorld* World = Avatar->GetWorld();
	if (!World) { return; }

	UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>();
	if (!Pool) { return; }

	// 발사 방향 — AIController 의 ControlRotation (Focus Task 가 SetFocus 로 타겟 향해 매 Tick 갱신, Z 포함)
	// 발사 위치는 임시 — Bow socket / 본 위치 정교화는 Montage 통합 단계
	const FRotator AimRotation = Avatar->GetControlRotation();
	const FVector Forward = AimRotation.Vector();
	const FVector SpawnLocation = Avatar->GetActorLocation() + Forward * 100.0f + FVector(0.0f, 0.0f, 50.0f);
	const FTransform SpawnTransform(AimRotation, SpawnLocation);

	ABOProjectile* Arrow = Cast<ABOProjectile>(Pool->SpawnFromPool(ArrowProjectileClass, SpawnTransform));
	if (!Arrow) { return; }

	Arrow->SetOwner(Avatar);
	Arrow->SetInstigator(Avatar);

	// GE Spec — Data.Damage SetByCaller 주입. ExecCalc_DamageCalc 가 그 값으로 Health 감소
	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, DamageMagnitude);
			Arrow->InitFromSpec(SpecHandle, /*Radius=*/ 0.0f);
		}
	}

	// 발사 Cue — Cue Notify BP 가 OnExecute 로 받아 VFX / SFX 재생
	if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SpawnLocation;
		CueParams.Normal = Forward;
		SourceASC->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Wraith_Fire, CueParams);
	}

	Arrow->Launch(Forward);
}
