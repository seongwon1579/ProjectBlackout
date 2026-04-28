// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/GA_BossTest.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Characters/BlackoutBossCharacter.h"
#include "MotionWarpingComponent.h"

UGA_BossTest::UGA_BossTest()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy.Attack1")));
}

void UGA_BossTest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("ActivateAbility"));

	// WarpTarget 설정: 몽타주 재생 전에 반드시 먼저 설정
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());

	// float PlayRate = 1.0f;
	// if (Boss && Boss->MotionWarpingComponent)
	// {
	// 	APawn* Target = Boss->GetWorld()->GetFirstPlayerController()->GetPawn();
	// 	if (Target)
	// 	{
	// 		// bFollowComponent=true: 매 프레임 타겟 위치를 추적 — 플레이어가 움직여도 따라감
	// 		Boss->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
	// 			WarpTargetName,
	// 			Target->GetRootComponent(),
	// 			NAME_None,
	// 			true,
	// 			FVector::ZeroVector
	// 		);
	// 	}
	// }

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.f,
		NAME_None,
		true,
		1.f
		);
	
	Task->OnCompleted.AddDynamic(this, &UGA_BossTest::OnMontageEnded);
	
	Task->ReadyForActivation();
}

void UGA_BossTest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// WarpTarget 정리
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());
	if (Boss && Boss->MotionWarpingComponent)
	{
		Boss->MotionWarpingComponent->RemoveWarpTarget(WarpTargetName);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossTest::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BossTest::OnMontageInterrupted()
{
}

void UGA_BossTest::OnMontageCancelled()
{
}
