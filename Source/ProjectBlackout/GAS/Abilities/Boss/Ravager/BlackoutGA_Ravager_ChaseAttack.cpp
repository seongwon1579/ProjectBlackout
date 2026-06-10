// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_ChaseAttack.h"

#include "BlackoutGameplayTags.h"
#include "BORavagerBoss.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tasks/AbilityTask_TickerTask.h"

void UBlackoutGA_Ravager_ChaseAttack::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::PreActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBlackoutGA_Ravager_ChaseAttack::SetupEventListeners()
{
	Super::SetupEventListeners();
	
	UAbilityTask_WaitGameplayEvent* WaitStart = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		BlackoutGameplayTags::Event_Enemy_Attack_KickFlash_Start,
		nullptr,
		true);
	WaitStart->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_ChaseAttack::OnMoveStart);
	WaitStart->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitEnd = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this, 
	BlackoutGameplayTags::Event_Enemy_Attack_KickFlash_End,
	nullptr,
	true);
	WaitEnd->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_ChaseAttack::OnMoveEnd);
	WaitEnd->ReadyForActivation();
}

void UBlackoutGA_Ravager_ChaseAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ABORavagerBoss* Ravager = Cast<ABORavagerBoss>(CachedOwner))
	{
		Ravager->SetCollisionState(false);
	}
	StopMove();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Ravager_ChaseAttack::OnMoveStart(FGameplayEventData Payload)
{
	if (!CanActivatePattern() ||!CachedOwner) return;
	
	if (ABORavagerBoss* Ravager = Cast<ABORavagerBoss>(CachedOwner))
	{
		Ravager->SetCollisionState(true);
	}
	
	MoveTarget = CachedTarget;
	if (!MoveTarget.IsValid()) return;
	
	MoveDuration = Payload.EventMagnitude;
	if (MoveDuration <= 0.f) return;
	
	MoveStartLocation = CachedOwner->GetActorLocation() - CachedOwner->GetActorForwardVector(); 
	MoveElapsed = 0.f;
	bMoving = true;
	
	const FVector TargetLoc = MoveTarget->GetActorLocation();
	FVector Dir = (MoveStartLocation - TargetLoc).GetSafeNormal2D();
	if (Dir.IsNearlyZero())
		Dir = -CachedOwner->GetActorForwardVector();
	GoalLocSmoothed = TargetLoc + Dir;
	GoalLocSmoothed.Z = MoveStartLocation.Z;

	MoveTask = UAbilityTask_TickerTask::CreateTickerTask(this);
	MoveTask->OnTick.AddDynamic(this, &UBlackoutGA_Ravager_ChaseAttack::OnMoveTick);
	MoveTask->ReadyForActivation();
	
}

void UBlackoutGA_Ravager_ChaseAttack::OnMoveEnd(FGameplayEventData Payload)
{
	StopMove();
}

void UBlackoutGA_Ravager_ChaseAttack::OnMoveTick(float DeltaTime)
{
	if (!bMoving || !CachedOwner || !MoveTarget.IsValid())
		return;

	MoveElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(MoveElapsed / MoveDuration, 0.f, 1.f);

	const FVector TargetLoc = MoveTarget->GetActorLocation();
	FVector Dir = (MoveStartLocation - TargetLoc).GetSafeNormal2D();
	if (Dir.IsNearlyZero())
		Dir = -CachedOwner->GetActorForwardVector();
	FVector GoalLoc = TargetLoc + Dir;
	GoalLoc.Z = MoveStartLocation.Z;

	GoalLocSmoothed = FMath::VInterpTo(GoalLocSmoothed, GoalLoc, DeltaTime, 8.f);

	const FVector NewLoc = FMath::Lerp(MoveStartLocation, GoalLocSmoothed, Alpha);

	FHitResult Hit;
	CachedOwner->SetActorLocation(NewLoc, false);

	if (Alpha >= 1.f)
	{
		StopMove();
	}
}

void UBlackoutGA_Ravager_ChaseAttack::StopMove()
{
	bMoving = false;
	
	if (MoveTask)
	{
		MoveTask->EndTask();
		MoveTask = nullptr;
	}
	MoveTarget.Reset();
}


