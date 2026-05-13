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

bool UBlackoutGA_Dodge::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	if (!DodgeData || !DodgeData->DodgeMontage)
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
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

	return true;
}

void UBlackoutGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Dodge activate requested");

	// 사전 조건(ActorInfo/DodgeData/스태미나/Blocked tags)은 CanActivateAbility 에서 이미 검증됨.
	// 여기서는 실제 활성에 필요한 cast 와 commit 만 수행합니다.
	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter || !ConsumeStamina())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: PlayerCharacter가 없거나 스태미나 차감 실패");
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

	// v2.3 (옵션 A — 클라 권위 위치 동기화):
	// - bIgnoreClientMovementErrorChecksAndCorrection: 서버가 ClientAdjustPosition RPC 를 보내지 않음 (jitter 차단).
	// - bServerAcceptClientAuthoritativePosition: 매 ServerMove 마다 서버가 클라 보고 위치를 그대로 채택.
	//   두 플래그 조합으로 root motion 진행 중 server↔client 위치가 자연 동기화 → EndAbility 시 워프 없음.
	// PvE 코옵 전제. EndAbility 에서 반드시 false 로 복구.
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = true;
		MovementComponent->bServerAcceptClientAuthoritativePosition = true;
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

			// v2.3: ActivateAbility 에서 켰던 클라 권위 동기화 플래그 복구.
			if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
			{
				MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = false;
				MovementComponent->bServerAcceptClientAuthoritativePosition = false;
			}
		}
	}

	ClearAllChainTimers();
	ResetChainState();

	if (ChainInputTask)
	{
		ChainInputTask->EndTask();
		ChainInputTask = nullptr;
	}

	if (MontageTask)
	{
		// bStopWhenAbilityEnds=true 로 만들었으므로 EndTask 만 호출하면 몽타주가 자연 정지.
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

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

	// v2.1: 양측 모두 체인 입력을 처리합니다. 클라이언트는 로컬 예측 점프를, 서버는 권위 점프를 수행합니다.
	ProcessChainInput();

	if (IsActive())
	{
		StartChainInputTask();
	}
}

void UBlackoutGA_Dodge::ProcessChainInput()
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
		StartChainedDodge();
		return;
	}

	if (bChainGraceWindowOpen || WasInputWithinChainGrace(InputPayload))
	{
		StartChainedDodge();
		return;
	}

	// 윈도우 도래 전 입력 → buffer 적재.
	BufferChainInput(InputPayload);
}

void UBlackoutGA_Dodge::ScheduleChainTimers()
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
		OnChainWindowOpenTimer();
	}
	else
	{
		TimerManager.SetTimer(
			ChainWindowOpenTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnChainWindowOpenTimer,
			OpenDelay,
			false);
	}

	if (CloseDelay > KINDA_SMALL_NUMBER)
	{
		TimerManager.SetTimer(
			ChainWindowCloseTimerHandle,
			this,
			&UBlackoutGA_Dodge::OnChainWindowCloseTimer,
			CloseDelay,
			false);
	}

	BO_LOG_GAS(Log,
		"GA_Dodge scheduled chain timers: Open=%.3f Close=%.3f Authority=%s",
		OpenDelay,
		CloseDelay,
		HasAuthority(&CurrentActivationInfo) ? TEXT("true") : TEXT("false"));
}

void UBlackoutGA_Dodge::OnChainWindowOpenTimer()
{
	bChainWindowOpen = true;
	bChainGraceWindowOpen = false;
	ChainWindowOpenedServerTime = GetCurrentServerTimeSeconds();
	ChainWindowClosedServerTime = 0.f;
	ActiveChainGraceDuration = 0.f;

	BO_LOG_GAS(Log,
		"GA_Dodge chain window opened: Authority=%s",
		HasAuthority(&CurrentActivationInfo) ? TEXT("true") : TEXT("false"));

	if (bChainInputQueued)
	{
		if (IsBufferedChainInputStillValid())
		{
			StartChainedDodge();
		}
		else
		{
			bChainInputQueued = false;
			bHasQueuedChainInputPayload = false;
			BO_LOG_GAS(Log, "GA_Dodge buffered chain input expired before window open");
		}
	}
}

void UBlackoutGA_Dodge::OnChainWindowCloseTimer()
{
	if (!bChainWindowOpen)
	{
		return;
	}

	bChainWindowOpen = false;
	ChainWindowClosedServerTime = GetCurrentServerTimeSeconds();

	if (bChainInputQueued)
	{
		StartChainedDodge();
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
			&UBlackoutGA_Dodge::OnChainGraceCloseTimer,
			ActiveChainGraceDuration,
			false);
	}

	BO_LOG_GAS(Log, "GA_Dodge chain grace started: Duration=%.3f", ActiveChainGraceDuration);
}

void UBlackoutGA_Dodge::OnChainGraceCloseTimer()
{
	bChainGraceWindowOpen = false;
	bChainInputQueued = false;
	bHasQueuedChainInputPayload = false;
	BO_LOG_GAS(Log, "GA_Dodge chain grace closed");
}

void UBlackoutGA_Dodge::OnChainInputBufferExpired()
{
	if (!bChainWindowOpen && !bChainGraceWindowOpen)
	{
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		BO_LOG_GAS(Log, "GA_Dodge chain input buffer expired");
	}
}

bool UBlackoutGA_Dodge::StartChainedDodge()
{
	ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return false;
	}

	const bool bIsAuthority = HasAuthority(&CurrentActivationInfo);

	// v2.1: 양측 모두 스태미나 체크 + 소모 (LocalPredicted 패턴, 초기 활성과 동일).
	// 클라이언트는 ApplyModToAttribute 로 로컬 attribute 를 예측 차감 → 부족하면 false 반환 → 예측 점프 안 함.
	// 서버 권위 attribute 가 자연 보정합니다.
	if (!ConsumeStamina())
	{
		BO_LOG_GAS(Log,
			"GA_Dodge chain skipped: 스태미나 부족 (Authority=%s)",
			bIsAuthority ? TEXT("true") : TEXT("false"));
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		return false;
	}

	if (!StartDodgeInternal(PlayerCharacter, /*bIsChainRestart=*/true))
	{
		BO_LOG_GAS(Warning, "GA_Dodge chain failed: 회피 재시작에 실패함");
		return false;
	}

	BO_LOG_GAS(Log,
		"GA_Dodge chain started: Authority=%s",
		bIsAuthority ? TEXT("true") : TEXT("false"));
	return true;
}

void UBlackoutGA_Dodge::BufferChainInput(const FBlackoutAbilityInputSyncPayload& InputPayload)
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
			&UBlackoutGA_Dodge::OnChainInputBufferExpired,
			DodgeData->ServerReceiveBufferDuration,
			false);
	}

	BO_LOG_GAS(Log, "GA_Dodge chain input buffered");
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
	K2_EndAbility();
}

void UBlackoutGA_Dodge::OnDodgeMontageInterrupted()
{
	BO_LOG_GAS(Log, "GA_Dodge montage interrupted");
	K2_EndAbility();
}

void UBlackoutGA_Dodge::OnDodgeMontageCancelled()
{
	BO_LOG_GAS(Log, "GA_Dodge montage cancelled");
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

	if (bIsChainRestart)
	{
		// v2: 체인 회피는 **동일한 PlayMontageAndWait 태스크 / montage instance** 안에서
		// 첫 섹션으로 position 만 0 으로 리셋합니다.
		const FName FirstSectionName = DodgeData->DodgeMontage->GetSectionName(0);
		if (FirstSectionName != NAME_None)
		{
			if (HasAuthority(&CurrentActivationInfo))
			{
				// 서버 권위: ASC::CurrentMontageJumpToSection 이 RepAnimMontageInfo Position 을 갱신해 자동 복제.
				if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
				{
					AbilitySystemComponent->CurrentMontageJumpToSection(FirstSectionName);
				}
			}
			else
			{
				// 클라이언트 로컬 예측: RPC 없이 AnimInstance 에 직접 jump.
				if (UAnimInstance* AnimInstance = PlayerCharacter->GetMesh() ? PlayerCharacter->GetMesh()->GetAnimInstance() : nullptr)
				{
					AnimInstance->Montage_JumpToSection(FirstSectionName, DodgeData->DodgeMontage);
				}
			}
		}
		else
		{
			BO_LOG_GAS(Warning,
				"GA_Dodge chain restart: 몽타주 %s 에 named section 이 없어 position 리셋이 무시됨",
				*GetNameSafe(DodgeData->DodgeMontage));
		}
	}
	else
	{
		// 첫 활성: 새 PlayMontageAndWait 태스크 시작.
		StartMontageTask();
	}

	CurrentDodgeStartedServerTime = GetCurrentServerTimeSeconds();

	// v2.1: 체인 윈도우/그레이스 타이머는 양측 모두 스케줄. 클라이언트는 로컬 예측에 사용합니다.
	ScheduleChainTimers();

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
