#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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

	// 태그의 동적 관리를 위해 ActivationOwnedTags가 아닌 코드 수동 추가로 전환합니다.
	// ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Locked);
	
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(BlackoutGameplayTags::Ability_Player_Dodge);
	SetAssetTags(AssetTags);

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
	if (!PlayerCharacter)
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: PlayerCharacter가 없음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanPayStaminaCost())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 스태미나 부족");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(PlayerCharacter))
	{
		BO_LOG_GAS(Log, "GA_Dodge cancelling active melee before dodge");
		MeleeAbility->K2_CancelAbility();
	}

	// 행동 차단 태그를 수동으로 부여합니다. (캔슬 윈도우 내 동적 격리를 위함)
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Locked);
		}
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

	LastProcessedChainInputSequenceId = 0;

	const FBlackoutAbilityInputSyncPayload DodgeInputPayload = GetLatestChainInputPayload();
	if (!StartDodgeInternal(PlayerCharacter, /*bIsChainRestart=*/false, &DodgeInputPayload))
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 시작에 실패함");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ConsumeStamina())
	{
		BO_LOG_GAS(Warning, "GA_Dodge failed: 회피 시작 후 스태미나 차감 실패");
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

	// 행동 차단 태그를 수동으로 제거 및 안전 회수합니다.
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Locked);
		}
	}

	ClearAllChainTimers();
	ResetChainState();
	LastProcessedChainInputSequenceId = 0;

	if (ChainInputTask)
	{
		ChainInputTask->EndTask();
		ChainInputTask = nullptr;
	}

	if (CancelableEventTask)
	{
		CancelableEventTask->EndTask();
		CancelableEventTask = nullptr;
	}

	if (MontageTask)
	{
		// bStopWhenAbilityEnds=true 로 만들었으므로 EndTask 만 호출하면 몽타주가 자연 정지.
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UBlackoutGA_Dodge::StartMontageTask()
{
	if (!DodgeData || !DodgeData->DodgeMontage)
	{
		return false;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		DodgeData->DodgeMontage,
		1.f,
		NAME_None,
		true,
		1.f,
		0.f,
		false);

	if (!MontageTask)
	{
		BO_LOG_GAS(Warning, "GA_Dodge montage task 생성 실패");
		return false;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageCancelled);
	MontageTask->OnBlendOut.AddDynamic(this, &UBlackoutGA_Dodge::OnDodgeMontageBlendOut);
	MontageTask->ReadyForActivation();
	return true;
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

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 서버만 체인 회피 재시작과 몽타주 위치 리셋을 확정합니다.
		ProcessChainInput();
	}
	else
	{
		// 클라이언트는 이미 ASC 입력 복제 경로로 서버에 입력을 보냈으므로,
		// 로컬 체인 재시작은 수행하지 않고 RepAnimMontageInfo 보정을 기다립니다.
		BO_LOG_GAS(Verbose, "GA_Dodge chain input forwarded to server");
	}

	if (IsActive())
	{
		StartChainInputTask();
	}
}

void UBlackoutGA_Dodge::StartCancelableEventTask()
{
	if (CancelableEventTask)
	{
		CancelableEventTask->EndTask();
		CancelableEventTask = nullptr;
	}

	CancelableEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Montage_AbilityCancelable,
		nullptr,
		false,
		false);

	if (CancelableEventTask)
	{
		CancelableEventTask->EventReceived.AddDynamic(this, &UBlackoutGA_Dodge::OnAbilityCancelableEventReceived);
		CancelableEventTask->ReadyForActivation();
	}
}

void UBlackoutGA_Dodge::OnAbilityCancelableEventReceived(FGameplayEventData Payload)
{
	BO_LOG_GAS(Log, "GA_Dodge: 캔슬 윈도우 오픈 (Event_Montage_AbilityCancelable 수신). 다른 행동의 시전을 대기합니다.");
	
	bCancelWindowOpen = true;

	// 행동 차단 태그를 동적으로 제거하여 다른 어빌리티가 활성화될 수 있게 허용합니다.
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Locked);
		}

		// 네트워크 시차로 인한 서버 예측 실패(Prediction Failed) 방지를 위해, 클라이언트 로컬에서 선제적으로 서버에도 태그 제거 RPC를 동기화하여 전송합니다.
		if (GetCurrentActorInfo() && GetCurrentActorInfo()->IsLocallyControlled())
		{
			if (UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent))
			{
				BlackoutASC->Server_RemoveLooseGameplayTag(BlackoutGameplayTags::State_Locked);
			}
		}
	}

	// 캔슬 윈도우가 열려 있는 동안 무브먼트(WASD) 입력을 실시간(0.02초 주기) 모니터링하기 시작합니다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CancelInputCheckTimerHandle,
			this,
			&UBlackoutGA_Dodge::CheckCancelInput,
			0.02f,
			true);
	}
}

void UBlackoutGA_Dodge::CheckCancelInput()
{
	if (!bCancelWindowOpen)
	{
		return;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		// 플레이어의 이동 입력(WASD)이 감지되면 즉시 몽타주를 정지하고 일반 무브먼트로 제어를 인계합니다.
		if (!PlayerCharacter->GetCachedMoveInput().IsNearlyZero())
		{
			BO_LOG_GAS(Log, "GA_Dodge: 캔슬 윈도우 내 이동 입력 감지. 몽타주를 강제 정지하고 어빌리티를 취소합니다.");

			if (DodgeData && DodgeData->DodgeMontage)
			{
				PlayerCharacter->StopAnimMontage(DodgeData->DodgeMontage);
			}

			K2_EndAbility();
		}
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
	LastProcessedChainInputSequenceId = InputPayload.SequenceId;

	if (bChainWindowOpen)
	{
		StartChainedDodge(InputPayload);
		return;
	}

	if (bChainGraceWindowOpen || WasInputWithinChainGrace(InputPayload))
	{
		StartChainedDodge(InputPayload);
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
			StartChainedDodge(QueuedChainInputPayload);
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
		StartChainedDodge(QueuedChainInputPayload);
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

bool UBlackoutGA_Dodge::StartChainedDodge(const FBlackoutAbilityInputSyncPayload& InputPayload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		BO_LOG_GAS(Verbose, "GA_Dodge chain ignored on client: 서버 권위 재시작 대기");
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		return false;
	}

	ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return false;
	}

	// 체인 회피는 서버에서만 스태미나를 확정 차감합니다.
	if (!CanPayStaminaCost())
	{
		BO_LOG_GAS(Log, "GA_Dodge chain skipped: 스태미나 부족");
		bChainInputQueued = false;
		bHasQueuedChainInputPayload = false;
		return false;
	}

	if (!StartDodgeInternal(PlayerCharacter, /*bIsChainRestart=*/true, &InputPayload))
	{
		BO_LOG_GAS(Warning, "GA_Dodge chain failed: 회피 재시작에 실패함");
		return false;
	}

	if (!ConsumeStamina())
	{
		BO_LOG_GAS(Warning, "GA_Dodge chain failed: 회피 재시작 후 스태미나 차감 실패");
		K2_EndAbility();
		return false;
	}

	BO_LOG_GAS(Log, "GA_Dodge chain started: Authority=true");
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

bool UBlackoutGA_Dodge::StartDodgeInternal(ABlackoutPlayerCharacter* PlayerCharacter, bool bIsChainRestart, const FBlackoutAbilityInputSyncPayload* InputPayload)
{
	if (!PlayerCharacter || !DodgeData || !DodgeData->DodgeMontage)
	{
		return false;
	}

	bool bIsBackstep = false;
	const FVector DodgeDirection = CalculateDodgeDirection(CurrentActorInfo, bIsBackstep, bIsChainRestart, InputPayload);
	if (DodgeDirection.IsNearlyZero())
	{
		BO_LOG_GAS(Warning, "GA_Dodge: 회피 방향 계산 결과가 0 벡터");
		return false;
	}

	// 현재 백스텝은 임시로 항상 forward roll. 향후 방향 입력 X 시 백스텝 분기 예정.
	bIsBackstep = false;

	FName FirstSectionName = NAME_None;
	if (bIsChainRestart)
	{
		if (!HasAuthority(&CurrentActivationInfo))
		{
			BO_LOG_GAS(Verbose, "GA_Dodge chain restart ignored on client: 서버 RepAnimMontageInfo 대기");
			return false;
		}

		FirstSectionName = DodgeData->DodgeMontage->GetSectionName(0);
		if (FirstSectionName == NAME_None)
		{
			BO_LOG_GAS(Warning,
				"GA_Dodge chain restart failed: 몽타주 %s 에 named section 이 없어 position 리셋 불가",
				*GetNameSafe(DodgeData->DodgeMontage));
			return false;
		}

		if (!GetAbilitySystemComponentFromActorInfo())
		{
			BO_LOG_GAS(Warning, "GA_Dodge chain restart failed: ASC가 비어 있음");
			return false;
		}
	}

	ResetChainState();
	ClearAllChainTimers();

	// 체인 재시작 등으로 인해 캔슬 윈도우 상태가 리셋될 경우 행동 차단 태그를 명시적으로 복구합니다.
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Locked);
		}
	}

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
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			// 서버 권위: ASC::CurrentMontageJumpToSection 이 RepAnimMontageInfo Position 을 갱신해 자동 복제.
			AbilitySystemComponent->CurrentMontageJumpToSection(FirstSectionName);
			PlayerCharacter->Multicast_SyncDodgeChainRestart(DodgeData->DodgeMontage, FirstSectionName, TargetRotation.Yaw);
			PlayerCharacter->Client_JumpMontageToSection(DodgeData->DodgeMontage, FirstSectionName, true, TargetRotation.Yaw);
		}
	}
	else
	{
		// 첫 활성: 새 PlayMontageAndWait 태스크 시작.
		if (!StartMontageTask())
		{
			return false;
		}
	}

	CurrentDodgeStartedServerTime = GetCurrentServerTimeSeconds();

	// 체인 윈도우/그레이스 타이머 예약. 체인 재시작 시에는 서버 권위 상태만 갱신됩니다.
	ScheduleChainTimers();

	// 캔슬 감지 태스크 활성화 (첫 실행 및 체인 재시작 공통)
	StartCancelableEventTask();

	return true;
}

bool UBlackoutGA_Dodge::CanPayStaminaCost() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent || !DodgeData)
	{
		return false;
	}

	const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(AbilitySystemComponent);
	const float StaminaCostMultiplier = BlackoutASC ? BlackoutASC->GetStaminaCostMultiplier() : 1.f;
	const float ModifiedStaminaCost = DodgeData->StaminaCost * StaminaCostMultiplier;
	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStaminaAttribute());
	return CurrentStamina >= ModifiedStaminaCost;
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
	if (!CanPayStaminaCost())
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

FVector UBlackoutGA_Dodge::CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep, bool bPreferControlForwardWhenNoInput, const FBlackoutAbilityInputSyncPayload* InputPayload) const
{
	bOutIsBackstep = false;

	const ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector2D PendingDodgeInput = PlayerCharacter->GetPendingDodgeInput();
	const FVector2D CachedMoveInput = PlayerCharacter->GetCachedMoveInput();
	const FVector2D PayloadMoveInput = (InputPayload && InputPayload->bHasMoveInput) ? InputPayload->MoveInput : FVector2D::ZeroVector;
	const FVector2D InputToUse = !PayloadMoveInput.IsNearlyZero()
		? PayloadMoveInput
		: (!PendingDodgeInput.IsNearlyZero() ? PendingDodgeInput : CachedMoveInput);
	const float ControlYaw = (InputPayload && InputPayload->bHasControlYaw)
		? InputPayload->ControlYawDegrees
		: PlayerCharacter->GetControlRotation().Yaw;

	BO_LOG_GAS(Log,
		"DodgeInput: Payload=(%.2f, %.2f), Pending=(%.2f, %.2f), Cached=(%.2f, %.2f), ControlYaw=%.2f",
		PayloadMoveInput.X, PayloadMoveInput.Y,
		PendingDodgeInput.X, PendingDodgeInput.Y,
		CachedMoveInput.X, CachedMoveInput.Y,
		ControlYaw);

	if (!InputToUse.IsNearlyZero())
	{
		const FRotator YawRotation(0.f, ControlYaw, 0.f);

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
		const FRotator YawRotation(0.f, ControlYaw, 0.f);
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

	if (LastProcessedChainInputSequenceId != 0)
	{
		const uint16 SequenceDelta = InputPayload.SequenceId - LastProcessedChainInputSequenceId;
		if (SequenceDelta == 0 || SequenceDelta >= 32768)
		{
			BO_LOG_GAS(Verbose, "GA_Dodge chain input rejected: 이미 처리한 sequence");
			return false;
		}
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
		TimerManager.ClearTimer(CancelInputCheckTimerHandle);
	}
}

void UBlackoutGA_Dodge::ResetChainState()
{
	bChainWindowOpen = false;
	bChainGraceWindowOpen = false;
	bChainInputQueued = false;
	bHasQueuedChainInputPayload = false;
	bCancelWindowOpen = false;
	ChainWindowOpenedServerTime = 0.f;
	ChainWindowClosedServerTime = 0.f;
	ActiveChainGraceDuration = 0.f;
}
