// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Boss/GA_BossTest.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "GAS/Tasks/AbilityTask_BossMeleeSweep.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"
#include "MotionWarpingComponent.h"

UGA_BossTest::UGA_BossTest()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(BlackoutGameplayTags::Character_Enemy_Attack1);
	SetAssetTags(Tags);
}

//TODO: 모든 기능을 한번에 넣었음 나중에 분리 필요
void UGA_BossTest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("ActivateAbility"));

	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());

	if (Boss && Boss->MotionWarpingComponent)
	{
		// TODO : ActorInfo에 플레이어를 사용 하는것으로 변경 필요
		APawn* Target = Boss->GetWorld()->GetFirstPlayerController()->GetPawn();
		if (Target)
		{
			Boss->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
				WarpTargetName,
				Target->GetRootComponent(),
				NAME_None,
				true,
				FVector::ZeroVector
			);
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.f,
		NAME_None,
		true,
		1.f
	);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_BossTest::OnMontageEnded);
	MontageTask->ReadyForActivation();

	// ANS Begin → sweep 시작
	UAbilityTask_WaitGameplayEvent* WaitStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_SweepStart,
		nullptr,
		true
	);
	WaitStartTask->EventReceived.AddDynamic(this, &UGA_BossTest::OnSweepStartEvent);
	WaitStartTask->ReadyForActivation();
}

void UGA_BossTest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
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

void UGA_BossTest::OnSweepStartEvent(FGameplayEventData Payload)
{
	ActiveSweepTask = UAbilityTask_BossMeleeSweep::CreateSweepTask(this, SweepStartSocket, SweepEndSocket, SweepRadius);
	ActiveSweepTask->OnHit.AddDynamic(this, &UGA_BossTest::OnMeleeSweepHit);
	ActiveSweepTask->ReadyForActivation();

	// ANS End → sweep 종료
	UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_SweepEnd,
		nullptr,
		true
	);
	WaitEndTask->EventReceived.AddDynamic(this, &UGA_BossTest::OnSweepEndEvent);
	WaitEndTask->ReadyForActivation();
}

void UGA_BossTest::OnSweepEndEvent(FGameplayEventData Payload)
{
	if (ActiveSweepTask)
	{
		ActiveSweepTask->EndTask();
		ActiveSweepTask = nullptr;
	}
}

void UGA_BossTest::OnMeleeSweepHit(const FHitResult& HitResult)
{
	if (!DamageEffectClass)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor);
	if (!Damageable)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		Damageable->ReceiveDamageFromHitbox(SpecHandle, HitResult.BoneName);
	}
}
