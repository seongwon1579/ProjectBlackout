// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Charge.h"

#include "BlackoutGameplayTags.h"
#include "BORavagerBoss.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tasks/AbilityTask_TickerTask.h"


void UBlackoutGA_Ravager_Charge::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
	
	CachedTargetComponent = TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBlackoutGA_Ravager_Charge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckTimerHandle);
	}
    
	if (ChargeTickTask)
	{
		ChargeTickTask->EndTask();
		ChargeTickTask = nullptr;
	}
    
	if (UCharacterMovementComponent* Move = CachedOwner ? CachedOwner->GetCharacterMovement() : nullptr)
	{
		Move->StopMovementImmediately();
	}
    
	CachedTargetComponent.Reset();
	ChargeStartTime = 0.f;
	bEndingTriggered = false;
    
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Ravager_Charge::SetupEventListeners()
{
	Super::SetupEventListeners();
	
	UAbilityTask_WaitGameplayEvent* WaitLoop = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this,
	BlackoutGameplayTags::Event_Enemy_Attack_Charge_LoopStart,
	nullptr,
	true);
	WaitLoop->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_Charge::HandleLoopStart);
	WaitLoop->ReadyForActivation();
}

TSubclassOf<UGameplayEffect> UBlackoutGA_Ravager_Charge::GetDamageEffect() const
{
	return CachedPatternData ? CachedPatternData->ChargeSettings.DamageEffect : nullptr;
}

float UBlackoutGA_Ravager_Charge::GetDamageMagnitude() const
{
	return CachedPatternData ? CachedPatternData->ChargeSettings.DamageMagnitude : 0.f;
}

const TArray<FName>& UBlackoutGA_Ravager_Charge::GetHitboxComponentNames() const
{
	if (!CachedPatternData) return GetEmptyHitboxNames();
	return CachedPatternData->ChargeSettings.HitboxComponentNames;
}


bool UBlackoutGA_Ravager_Charge::HasValidSettings() const
{
	return CachedPatternData->ChargeSettings.IsValid();
}

void UBlackoutGA_Ravager_Charge::HandleLoopStart(FGameplayEventData Payload)
{
	if (!CanActivatePattern()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	ChargeStartTime = World->GetTimeSeconds();

	const float Interval = CachedPatternData->ChargeSettings.CheckInterval;
	World->GetTimerManager().SetTimer(
	   CheckTimerHandle, this, &UBlackoutGA_Ravager_Charge::TickCheckDistance, Interval, true);

	ChargeTickTask = UAbilityTask_TickerTask::CreateTickerTask(this);
	ChargeTickTask->OnTick.AddDynamic(this, &UBlackoutGA_Ravager_Charge::TickChargeMovement);
	ChargeTickTask->ReadyForActivation();
}

void UBlackoutGA_Ravager_Charge::TickChargeMovement(float DeltaTime)
{
	// if (bEndingTriggered || !CachedOwner) return;
	//
	// FVector Forward = CachedOwner->GetActorForwardVector();
	// Forward.Z = 0.f;
	// if (!Forward.Normalize()) return;
	//
	// CachedOwner->AddMovementInput(Forward, 1.f);
	//
	// if (UCharacterMovementComponent* Move = CachedOwner->GetCharacterMovement())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[Charge] MaxSpeed=%.1f / Mode=%d / Vel=%.1f / RootMotion=%d"),
	// 		Move->MaxWalkSpeed,
	// 		(int32)Move->MovementMode,
	// 		CachedOwner->GetVelocity().Size(),
	// 		Move->HasRootMotionSources() || Move->CurrentRootMotion.HasActiveRootMotionSources());
	// }
	if (bEndingTriggered || !CachedOwner || !CachedPatternData) return;

	FVector Forward = CachedOwner->GetActorForwardVector();
	Forward.Z = 0.f;
	if (!Forward.Normalize()) return;

	const float Speed = CachedPatternData->ChargeSettings.ChargeSpeed;
	const FVector Delta = Forward * Speed * DeltaTime;

	FHitResult Hit;
	CachedOwner->AddActorWorldOffset(Delta, true, &Hit);
}

void UBlackoutGA_Ravager_Charge::TickCheckDistance()
{
	if (bEndingTriggered || !CachedOwner || !CachedTargetComponent.IsValid() || !CanActivatePattern()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FBossChargeSettings& ChargeSetting = CachedPatternData->ChargeSettings;
	const float ElapsedTime = World->GetTimeSeconds() - ChargeStartTime;
	bool bShouldEnd = false;

	const float Dist2D = FVector::DistXY(CachedOwner->GetActorLocation(), CachedTargetComponent->GetComponentLocation());
	
	UE_LOG(LogTemp, Log, TEXT("[Charge] Dist2D=%.1f / StopDist=%.1f / Elapsed=%.2f"),
	   Dist2D, ChargeSetting.ChargeStopDistance, ElapsedTime);
	if (Dist2D <= ChargeSetting.ChargeStopDistance)
	{
		bShouldEnd = true;
	}

	if (ChargeSetting.MaxChargeDuration > 0.f && ElapsedTime >= ChargeSetting.MaxChargeDuration)
	{
		bShouldEnd = true;
	}

	if (bShouldEnd)
	{
		GoToEndSection();
	}
}

void UBlackoutGA_Ravager_Charge::GoToEndSection()
{
	if (bEndingTriggered || !CanActivatePattern()) return;
	bEndingTriggered = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckTimerHandle);
	}

	if (ChargeTickTask)
	{
		ChargeTickTask->EndTask();
		ChargeTickTask = nullptr;
	}

	if (UCharacterMovementComponent* Move = CachedOwner ? CachedOwner->GetCharacterMovement() : nullptr)
	{
		Move->StopMovementImmediately();
	}

	if (CachedOwner && SelectedMontage)
	{
		if (USkeletalMeshComponent* Mesh = CachedOwner->GetMesh())
		{
			if (UAnimInstance* Anim = Mesh->GetAnimInstance())
			{
				const FBossChargeSettings& ChargeSetting = CachedPatternData->ChargeSettings;
				Anim->Montage_JumpToSection(ChargeSetting.EndSectionName, SelectedMontage);
			}
		}
	}
}
