#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "Data/BODodgeData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "TimerManager.h"

UBlackoutGA_Dodge::UBlackoutGA_Dodge()
{
	InputID = EBlackoutAbilityInputID::Dodge;

	// i-frame 을 위한 임시 제거
	// ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Invulnerable);

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Locked);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_Dodge::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
}

void UBlackoutGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Dodge activate requested");

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: ActorInfo 또는 AvatarActor가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!DodgeData || !DodgeData->DodgeMontage)
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: DodgeData 또는 DodgeMontage 가 비어 있음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter || !ConsumeStamina())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: PlayerCharacter가 없거나 스태미나가 부족함");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(PlayerCharacter))
	{
		BO_LOG_GAS(Log, "GA_Dodge cancelling active melee before dodge");
		MeleeAbility->K2_CancelAbility();
	}

	if (!StartDodgeInternal(PlayerCharacter, /*bIsChainRestart=*/false))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 시작에 실패함");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 입력 수집은 양측 모두에서. 클라이언트는 표준 GAS 경로로 서버에 InputPressed 를 전파합니다.
	StartChainInputTask();
}

void UBlackoutGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Dodge ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->SetPendingDodgeInput(FVector2D::ZeroVector);
			PlayerCharacter->SetDodgeMontagePlaying(false);
		}

		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DodgeEndTimerHandle);
		}
	}

	ClearAllChainTimers();
	ResetChainState();

	if (ChainInputTask)
	{
		ChainInputTask->EndTask();
		ChainInputTask = nullptr;
	}

	EndMontageTaskQuietly();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Dodge::StartMontageTask()
{
	if (!DodgeData || !DodgeData->DodgeMontage)
	{
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		DodgeData->DodgeMontage,
		1.f,
		NAME_None,
		/*bStopWhenAbilityEnds=*/true,
		/*AnimRootMotionTranslationScale=*/1.f,
		/*StartTimeSeconds=*/0.f,
		/*bAllowInterruptAfterBlendOut=*/false);

	if (!MontageTask)
	{
		BO_LOG_GAS(Warning, "GA_Dodge montage task 생성 실패");
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCancelled);
	MontageTask->OnBlendOut.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageBlendOut);
	MontageTask->ReadyForActivation();
}

void UBlackoutGA_Dodge::EndMontageTaskQuietly()
{
	if (!MontageTask)
	{
		return;
	}

	MontageTask->OnCompleted.RemoveDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCompleted);
	MontageTask->OnInterrupted.RemoveDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageInterrupted);
	MontageTask->OnCancelled.RemoveDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCancelled);
	MontageTask->OnBlendOut.RemoveDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageBlendOut);
	MontageTask->EndTask();
	MontageTask = nullptr;
}

void UBlackoutGA_Dodge::StartChainInputTask()
{
	if (!IsActive())
	{
		return;
	}

	ChainInputTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	if (!ChainInputTask)
	{
		BO_LOG_GAS(Warning, "GA_Dodge chain input task 생성 실패");
		return;
	}

	ChainInputTask->OnPress.AddDynamic(this, &UBlackoutGA_Dodge::OnChainInputPressed);
	ChainInputTask->ReadyForActivation();
}

void UBlackoutGA_Dodge::OnChainInputPressed(float TimeWaited)
{
	ChainInputTask = nullptr;

	// v2: 체인 상태머신은 서버 권위. 클라이언트는 입력 알림만 받고 별도 처리 없음.
	if (HasAuthority(&CurrentActivationInfo))
	{
		ServerProcessChainInput();
	}

	if (IsActive())
	{
		StartChainInputTask();
	}
}

void UBlackoutGA_Dodge::ServerProcessChainInput()
{
	if (!DodgeData)
	{
		return;
	}

	const FBlackoutAbilityInputSyncPayload InputPayload = GetLatestChainInputPayload();
	if (!IsChainInputPayloadUsable(InputPayload))
	{
		return;
	}

	if (bChainWindowOpen)
	{
		ServerStartChainedDodge();
		return;
	}

	if (bChainGraceWindowOpen || WasInputWithinChainGrace(InputPayload))
	{
		ServerStartChainedDodge();
		return;
	}

	// 윈도우 도래 전 입력 → buffer 적재.
	ServerBufferChainInput(InputPayload);
}

void UBlackoutGA_Dodge::ServerScheduleChainTimers()
{
	if (!DodgeData)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(ChainWindowOpenTimerHandle);
	TimerManager.ClearTimer(ChainWindowCloseTimerHandle);
	TimerManager.ClearTimer(ChainGraceCloseTimerHandle);
	TimerManager.ClearTimer(ChainInputBufferTimerHandle);

	bChainWindowOpen = false;
	bChainGraceWindowOpen = false;
	ChainWindowOpenedServerTime = 0.f;
	ChainWindowClosedServerTime = 0.f;
	ActiveChainGraceDuration = 0.f;

	const float OpenDelay = FMath::Max(0.f, DodgeData->ChainWindowOpenAtSeconds);
	const float CloseDelay = FMath::Max(OpenDelay, DodgeData->ChainWindowCloseAtSeconds);

	if (OpenDelay <= KINDA_SMALL_NUMBER)
	{
		OnServerChainWindowOpenTimer();
	}
	else
	{
		TimerManager.SetTimer(
			ChainWindowOpenTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnServerChainWindowOpenTimer,
			OpenDelay,
			false);
	}

	if (CloseDelay > KINDA_SMALL_NUMBER)
	{
		TimerManager.SetTimer(
			ChainWindowCloseTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnServerChainWindowCloseTimer,
			CloseDelay,
			false);
	}

	BO_LOG_GAS(Log,
		"GA_Dodge scheduled chain timers (server): Open=%.3f Close=%.3f",
		OpenDelay,
		CloseDelay);
}

void UBlackoutGA_Dodge::OnServerChainWindowOpenTimer()
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	bChainWindowOpen = true;
	bChainGraceWindowOpen = false;
	ChainWindowOpenedServerTime = GetCurrentServerTimeSeconds();
	ChainWindowClosedServerTime = 0.f;
	ActiveChainGraceDuration = 0.f;

	BO_LOG_GAS(Log, "GA_Dodge chain window opened (server)");

	if (bChainInputQueued)
	{
		if (IsBufferedChainInputStillValid())
		{
			ServerStartChainedDodge();
		}
		else
		{
			bChainInputQueued = false;
			bHasQueuedChainInputPayload = false;
			BO_LOG_GAS(Log, "GA_Dodge buffered chain input expired before window open");
		}
	}
}

void UBlackoutGA_Dodge::OnServerChainWindowCloseTimer()
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	if (!bChainWindowOpen)
	{
		return;
	}

	bChainWindowOpen = false;
	ChainWindowClosedServerTime = GetCurrentServerTimeSeconds();

	if (bChainInputQueued)
	{
		ServerStartChainedDodge();
		return;
	}

	ActiveChainGraceDuration = GetDynamicChainGraceDuration();
	if (ActiveChainGraceDuration <= 0.f)
	{
		BO_LOG_GAS(Log, "GA_Dodge chain window closed without grace");
		return;
	}

	bChainGraceWindowOpen = true;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ChainGraceCloseTimerHandle);
		TimerManager.SetTimer(
			ChainGraceCloseTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnServerChainGraceCloseTimer,
			ActiveChainGraceDuration,
			false);
	}

	BO_LOG_GAS(Log,
		"GA_Dodge chain grace started (server): Duration=%.3f",
		ActiveChainGraceDuration);
}

void UBlackoutGA_Dodge::OnServerChainGraceCloseTimer()
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	bChainGraceWindowOpen = false;
	bChainInputQueued = false;
	bHasQueuedChainInputPayload = false;
	BO_LOG_GAS(Log, "GA_Dodge chain grace closed (server)");
}

void UBlackoutGA_Dodge::OnServerChainInputBufferExpired()
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	if (!bChainWindowOpen && !bChainGraceWindowOpen)
	{
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		BO_LOG_GAS(Log, "GA_Dodge chain input buffer expired (server)");
	}
}

bool UBlackoutGA_Dodge::ServerStartChainedDodge()
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return false;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return false;
	}

	if (!ConsumeStamina())
	{
		BO_LOG_GAS(Log, "GA_Dodge chain skipped: 스태미나 부족");
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		return false;
	}

	if (!StartDodgeInternal(PlayerCharacter, /*bIsChainRestart=*/true))
	{
		BO_LOG_GAS(Warning, "GA_Dodge chain failed: 회피 재시작에 실패함");
		return false;
	}

	BO_LOG_GAS(Log, "GA_Dodge chain started (server)");
	return true;
}

void UBlackoutGA_Dodge::ServerBufferChainInput(const FBlackoutAbilityInputSyncPayload& InputPayload)
{
	if (!DodgeData || DodgeData->ServerReceiveBufferDuration <= 0.f)
	{
		return;
	}

	bChainInputQueued = true;
	bHasQueuedChainInputPayload = true;
	QueuedChainInputPayload = InputPayload;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ChainInputBufferTimerHandle);
		TimerManager.SetTimer(
			ChainInputBufferTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnServerChainInputBufferExpired,
			DodgeData->ServerReceiveBufferDuration,
			false);
	}

	BO_LOG_GAS(Log, "GA_Dodge chain input buffered (server)");
}

UBlackoutGA_Dodge* UBlackoutGA_Dodge::GetActiveDodgeAbilityFromActor(const AActor* OwnerActor)
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
	const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			if (UBlackoutGA_Dodge* DodgeAbility = Cast<UBlackoutGA_Dodge>(AbilityInstance))
			{
				return DodgeAbility;
			}
		}
	}

	return nullptr;
}

void UBlackoutGA_Dodge::HandleChainWindowOpened()
{
	// v2: 체인 상태머신 권위에서 분리. 시각 effect 트리거가 필요한 경우 여기서 처리하세요.
	BO_LOG_GAS(Verbose, "GA_Dodge chain window notify (FX-only, v2)");
}

void UBlackoutGA_Dodge::OnDodgeMontageCompleted()
{
	BO_LOG_GAS(Log, "GA_Dodge montage completed");
	if (bChainRestarting)
	{
		return;
	}
	K2_EndAbility();
}

void UBlackoutGA_Dodge::OnDodgeMontageInterrupted()
{
	BO_LOG_GAS(Log, "GA_Dodge montage interrupted");
	if (bChainRestarting)
	{
		return;
	}
	K2_EndAbility();
}

void UBlackoutGA_Dodge::OnDodgeMontageCancelled()
{
	BO_LOG_GAS(Log, "GA_Dodge montage cancelled");
	if (bChainRestarting)
	{
		return;
	}
	K2_EndAbility();
}

void UBlackoutGA_Dodge::OnDodgeMontageBlendOut()
{
	BO_LOG_GAS(Verbose, "GA_Dodge montage blend out");
}

bool UBlackoutGA_Dodge::StartDodgeInternal(ABlackoutPlayerCharacter* PlayerCharacter, bool bIsChainRestart)
{
	if (!PlayerCharacter || !DodgeData || !DodgeData->DodgeMontage)
	{
		return false;
	}

	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(CurrentActorInfo, bIsBackstep, bIsChainRestart);
	if (DodgeDirection.IsNearlyZero())
	{
		BO_LOG_GAS(Warning, "GA_Dodge: 회피 방향 계산 결과가 0 벡터");
		return false;
	}

	// 현재 백스텝은 임시로 항상 forward roll. 향후 방향 입력 X 시 백스텝 분기 예정.
	bIsBackstep = false;

	// 체인 재시작 시 옛 MontageTask 의 콜백이 GA 를 끝내지 못하도록 가드 활성화.
	if (bIsChainRestart)
	{
		bChainRestarting = true;
		EndMontageTaskQuietly();
	}

	ResetChainState();
	ClearAllChainTimers();

	const FRotator TargetRotation(0.f, DodgeDirection.Rotation().Yaw, 0.f);
	PlayerCharacter->SetActorRotation(TargetRotation);
	PlayerCharacter->SetDodgeMontagePlaying(true);

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		CombatComponent->StopAim();
	}

	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	// v2: 몽타주 재생은 GAS 표준 task. ASC.PlayMontage 가 FRepAnimMontageInfo 를 갱신하여
	// 시뮬레이트 프록시/오너 클라이언트에 자동 복제.
	StartMontageTask();

	const float LaunchStrength = bIsBackstep ? DodgeData->BackstepStrength : DodgeData->DodgeStrength;
	PlayerCharacter->LaunchCharacter(
		DodgeDirection * LaunchStrength + FVector::UpVector * DodgeData->UpwardImpulse,
		true,
		true);

	CurrentDodgeStartedServerTime = GetCurrentServerTimeSeconds();

	if (HasAuthority(&CurrentActivationInfo))
	{
		ServerScheduleChainTimers();
	}

	const float DodgeEndDelay =
		(DodgeData->DodgeMontage->GetPlayLength() > 0.f)
			? DodgeData->DodgeMontage->GetPlayLength()
			: DodgeData->DodgeDuration;

	if (UWorld* World = PlayerCharacter->GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(DodgeEndTimerHandle);
		TimerManager.SetTimer(
			DodgeEndTimerHandle,
			[this]()
			{
				if (!bChainRestarting && IsActive())
				{
					BO_LOG_GAS(Log, "GA_Dodge finished by timer");
					K2_EndAbility();
				}
			},
			DodgeEndDelay,
			false);
	}

	if (bIsChainRestart)
	{
		bChainRestarting = false;
	}

	return true;
}

bool UBlackoutGA_Dodge::ConsumeStamina() const
{
	// TODO(stamina-cost): TDD §4.1 v2 는 GE Cost 사용을 명시. 후속 PR 에서 GE 기반으로 교체 예정.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent || !DodgeData)
	{
		return false;
	}

	const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent);
	const float StaminaCostMultiplier = BlackoutASC ? BlackoutASC->GetStaminaCostMultiplier() : 1.f;
	const float ModifiedStaminaCost = DodgeData->StaminaCost * StaminaCostMultiplier;
	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < ModifiedStaminaCost)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(
		UBlackoutPlayerAttributeSet::GetStaminaAttribute(),
		EGameplayModOp::Additive,
		-ModifiedStaminaCost);

	if (UBlackoutAbilitySystemComponent* BlackoutAbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent))
	{
		BlackoutAbilitySystemComponent->NotifyStaminaSpent();
	}

	return true;
}

FVector UBlackoutGA_Dodge::CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep, bool bPreferControlForwardWhenNoInput) const
{
	bOutIsBackstep = false;

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector2D PendingDodgeInput = PlayerCharacter->GetPendingDodgeInput();
	const FVector2D CachedMoveInput = PlayerCharacter->GetCachedMoveInput();
	const FVector2D InputToUse = !PendingDodgeInput.IsNearlyZero() ? PendingDodgeInput : CachedMoveInput;

	BO_LOG_GAS(Log,
		"DodgeInput: Pending=(%.2f, %.2f), Cached=(%.2f, %.2f)",
		PendingDodgeInput.X, PendingDodgeInput.Y,
		CachedMoveInput.X, CachedMoveInput.Y);

	if (!InputToUse.IsNearlyZero())
	{
		const FRotator ControlRotation = PlayerCharacter->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector ControlForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector ControlRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector WorldInputDirection =
			(ControlForward * InputToUse.Y + ControlRight * InputToUse.X).GetSafeNormal2D();

		if (!WorldInputDirection.IsNearlyZero())
		{
			return WorldInputDirection;
		}
	}

	if (bPreferControlForwardWhenNoInput)
	{
		const FRotator ControlRotation = PlayerCharacter->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	}

	const FVector LastMovementDirection = PlayerCharacter->GetLastMovementInputVector().GetSafeNormal2D();
	if (!LastMovementDirection.IsNearlyZero())
	{
		return LastMovementDirection;
	}

	const FVector VelocityDirection = PlayerCharacter->GetVelocity().GetSafeNormal2D();
	if (!VelocityDirection.IsNearlyZero())
	{
		return VelocityDirection;
	}

	return PlayerCharacter->GetActorForwardVector().GetSafeNormal2D();
}

bool UBlackoutGA_Dodge::IsChainInputPayloadUsable(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (!DodgeData || !InputPayload.IsValid())
	{
		return false;
	}

	if (InputPayload.AbilitySpecHandle.IsValid() && InputPayload.AbilitySpecHandle != GetCurrentAbilitySpecHandle())
	{
		return false;
	}

	const float ServerReceivedTime = InputPayload.ServerReceivedTimeSeconds > 0.f
		? InputPayload.ServerReceivedTimeSeconds
		: GetCurrentServerTimeSeconds();
	const float RawInputServerTime = InputPayload.ClientEstimatedServerTimeSeconds > 0.f
		? InputPayload.ClientEstimatedServerTimeSeconds
		: ServerReceivedTime;

	if (RawInputServerTime > ServerReceivedTime + DodgeData->MaxChainInputFutureTolerance)
	{
		BO_LOG_GAS(Log, "GA_Dodge chain input rejected: 미래 timestamp 허용 범위 초과");
		return false;
	}

	if (ServerReceivedTime - RawInputServerTime > DodgeData->MaxChainInputTimestampAge)
	{
		BO_LOG_GAS(Log, "GA_Dodge chain input rejected: timestamp age 초과");
		return false;
	}

	return true;
}

bool UBlackoutGA_Dodge::WasInputWithinChainGrace(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (ChainWindowClosedServerTime <= 0.f || ActiveChainGraceDuration <= 0.f)
	{
		return false;
	}

	const float InputServerTime = GetInputServerTimeSeconds(InputPayload);
	return InputServerTime >= ChainWindowClosedServerTime - KINDA_SMALL_NUMBER
		&& InputServerTime <= ChainWindowClosedServerTime + ActiveChainGraceDuration;
}

bool UBlackoutGA_Dodge::IsBufferedChainInputStillValid() const
{
	if (!bHasQueuedChainInputPayload || !DodgeData)
	{
		return true;
	}

	const float InputServerTime = GetInputServerTimeSeconds(QueuedChainInputPayload);
	const float WindowOpenTime = ChainWindowOpenedServerTime > 0.f
		? ChainWindowOpenedServerTime
		: GetCurrentServerTimeSeconds();
	const float Age = WindowOpenTime - InputServerTime;
	if (Age < -KINDA_SMALL_NUMBER)
	{
		return true;
	}
	return Age <= DodgeData->ServerReceiveBufferDuration + KINDA_SMALL_NUMBER;
}

float UBlackoutGA_Dodge::GetDynamicChainGraceDuration() const
{
	if (!DodgeData)
	{
		return 0.f;
	}

	const ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	const APlayerState* PlayerState = PlayerCharacter ? PlayerCharacter->GetPlayerState() : nullptr;
	const float PingSeconds = PlayerState ? PlayerState->GetPingInMilliseconds() * 0.001f : 0.f;
	const float GraceDuration =
		DodgeData->BaseChainInputGraceDuration
		+ PingSeconds * DodgeData->ChainInputPingScale
		+ DodgeData->ChainInputJitterMargin;
	return FMath::Clamp(GraceDuration, 0.f, DodgeData->MaxChainInputGraceDuration);
}

float UBlackoutGA_Dodge::GetCurrentServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

float UBlackoutGA_Dodge::GetInputServerTimeSeconds(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (!DodgeData)
	{
		return GetCurrentServerTimeSeconds();
	}

	const float ServerReceivedTime = InputPayload.ServerReceivedTimeSeconds > 0.f
		? InputPayload.ServerReceivedTimeSeconds
		: GetCurrentServerTimeSeconds();

	if (InputPayload.ClientEstimatedServerTimeSeconds <= 0.f)
	{
		return ServerReceivedTime;
	}

	return FMath::Clamp(
		InputPayload.ClientEstimatedServerTimeSeconds,
		ServerReceivedTime - DodgeData->MaxChainInputTimestampAge,
		ServerReceivedTime + DodgeData->MaxChainInputFutureTolerance);
}

FBlackoutAbilityInputSyncPayload UBlackoutGA_Dodge::GetLatestChainInputPayload() const
{
	if (const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		if (const FBlackoutAbilityInputSyncPayload* Payload = BlackoutASC->GetLatestInputSyncPayload(EBlackoutAbilityInputID::Dodge))
		{
			return *Payload;
		}
	}

	FBlackoutAbilityInputSyncPayload FallbackPayload;
	FallbackPayload.SequenceId = 1;
	FallbackPayload.InputID = EBlackoutAbilityInputID::Dodge;
	FallbackPayload.AbilitySpecHandle = GetCurrentAbilitySpecHandle();
	FallbackPayload.ClientEstimatedServerTimeSeconds = GetCurrentServerTimeSeconds();
	FallbackPayload.ServerReceivedTimeSeconds = FallbackPayload.ClientEstimatedServerTimeSeconds;
	return FallbackPayload;
}

void UBlackoutGA_Dodge::ClearAllChainTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ChainWindowOpenTimerHandle);
		TimerManager.ClearTimer(ChainWindowCloseTimerHandle);
		TimerManager.ClearTimer(ChainGraceCloseTimerHandle);
		TimerManager.ClearTimer(ChainInputBufferTimerHandle);
	}
}

void UBlackoutGA_Dodge::ResetChainState()
{
	bChainWindowOpen = false;
	bChainGraceWindowOpen = false;
	bChainInputQueued = false;
	bHasQueuedChainInputPayload = false;
	ChainWindowOpenedServerTime = 0.f;
	ChainWindowClosedServerTime = 0.f;
	ActiveChainGraceDuration = 0.f;
}
