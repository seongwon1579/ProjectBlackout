#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "Core/BlackoutLog.h"
#include "Data/BOMeleeComboData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "TimerManager.h"

UBlackoutGA_MeleePlayer::UBlackoutGA_MeleePlayer()
{
	InputID = EBlackoutAbilityInputID::Melee;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Attacking);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Aiming);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
}

void UBlackoutGA_MeleePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer activate requested");

	// 사전 조건(ActorInfo/MeleeComboData/첫 섹션/Blocked tags)은 CanActivateAbility 에서 검증됨.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer activated: Character=%s", *GetNameSafe(ActorInfo->AvatarActor.Get()));

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
		{
			CombatComponent->BeginMeleeWeaponAttachmentOverride();
			CombatComponent->StopAim();
		}

		// v2.3 (옵션 A — 클라 권위 위치 동기화):
		// - bIgnoreClientMovementErrorChecksAndCorrection: 서버 ClientAdjustPosition RPC 차단 (jitter 방지).
		// - bServerAcceptClientAuthoritativePosition: 매 ServerMove 마다 서버가 클라 위치를 채택.
		//   콤보 섹션 점프 timing 차이로 인한 누적 위치 오차가 자연 동기화되어 EndAbility 시 워프 방지.
		// PvE 코옵 전제. EndAbility 에서 반드시 false 로 복구.
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = true;
			MovementComponent->bServerAcceptClientAuthoritativePosition = true;
		}
	}

	ResetComboState();
	CurrentComboIndex = 0;

	const FBlackoutComboSectionDef& StartSection = MeleeComboData->ComboSections[0];

	SnapToControlYaw();

	// v2: 몽타주 재생은 GAS 표준 PlayMontageAndWait 태스크로 일원화.
	// 양측(서버/클라)이 동일한 ASC.PlayMontage 경로를 사용 → FRepAnimMontageInfo 가 시뮬레이트 프록시에 자동 복제.
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeComboData->MeleeMontage,
		1.f,
		StartSection.SectionName,
		true,
		1.f,
		0.f,
		false);

	if (!MontageTask)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer failed: PlayMontageAndWait 태스크 생성 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_MeleePlayer::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_MeleePlayer::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_MeleePlayer::OnMontageCancelled);
	MontageTask->OnBlendOut.AddDynamic(this, &UBlackoutGA_MeleePlayer::OnMontageBlendOut);
	MontageTask->ReadyForActivation();

	// v2.1: 양측(서버+클라이언트) 모두 콤보 윈도우/그레이스 타이머를 동일하게 스케줄합니다.
	// 같은 server world time + 같은 DA 시각값을 사용하므로 양쪽의 윈도우 상태가 거의 동기화됩니다.
	// 클라이언트의 윈도우는 입력 전파 보조 상태로만 유지하고, 섹션 전환은 서버 복제를 따릅니다.
	ScheduleSectionTimers(0);

	// 입력 수집은 양측 모두에서. 클라이언트는 표준 GAS 경로로 서버에 InputPressed 를 전파합니다.
	StartComboInputTask();
}

void UBlackoutGA_MeleePlayer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_MeleePlayer ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
			{
				CombatComponent->EndMeleeAttackWindow();
				CombatComponent->EndMeleeWeaponAttachmentOverride();
			}

			// v2.3: ActivateAbility 에서 켰던 클라 권위 동기화 플래그 복구.
			if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
			{
				MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = false;
				MovementComponent->bServerAcceptClientAuthoritativePosition = false;
			}
		}
	}

	ClearAllComboTimers();
	ResetComboState();

	if (ComboInputTask)
	{
		ComboInputTask->EndTask();
		ComboInputTask = nullptr;
	}

	if (MontageTask)
	{
		// bStopWhenAbilityEnds=true 로 만들었으므로 EndTask 만 호출하면 몽타주가 자연 정지됩니다.
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_MeleePlayer::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
}

bool UBlackoutGA_MeleePlayer::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	if (!MeleeComboData || !MeleeComboData->MeleeMontage || MeleeComboData->ComboSections.Num() == 0)
	{
		return false;
	}

	const FBlackoutComboSectionDef& StartSection = MeleeComboData->ComboSections[0];
	if (StartSection.SectionName == NAME_None
		|| MeleeComboData->MeleeMontage->GetSectionIndex(StartSection.SectionName) == INDEX_NONE)
	{
		return false;
	}

	return true;
}

void UBlackoutGA_MeleePlayer::StartComboInputTask()
{
	if (!IsActive())
	{
		return;
	}

	ComboInputTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	if (!ComboInputTask)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer combo input task 생성 실패");
		return;
	}

	ComboInputTask->OnPress.AddDynamic(this, &UBlackoutGA_MeleePlayer::OnComboInputPressed);
	ComboInputTask->ReadyForActivation();
}

void UBlackoutGA_MeleePlayer::OnComboInputPressed(float TimeWaited)
{
	ComboInputTask = nullptr;

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 서버만 콤보 상태와 몽타주 섹션 전환을 확정합니다.
		ProcessComboInput();
	}
	else
	{
		// 클라이언트는 이미 ASC 입력 복제 경로로 서버에 입력을 보냈으므로,
		// 로컬 섹션 점프는 수행하지 않고 RepAnimMontageInfo 보정을 기다립니다.
		BO_LOG_GAS(Verbose, "GA_MeleePlayer combo input forwarded to server");
	}

	if (IsActive())
	{
		StartComboInputTask();
	}
}

void UBlackoutGA_MeleePlayer::ProcessComboInput()
{
	if (!MeleeComboData)
	{
		return;
	}

	const FBlackoutAbilityInputSyncPayload InputPayload = GetLatestComboInputPayload();
	if (!IsComboInputPayloadUsable(InputPayload))
	{
		return;
	}
	LastProcessedComboInputSequenceId = InputPayload.SequenceId;

	// 순환 콤보 적용: 마지막 콤보 단계 이후에는 다시 첫 번째 단계(인덱스 0)로 순환하여 진행합니다.
	if (MeleeComboData->ComboSections.Num() == 0)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer combo failed: ComboSections가 비어 있음");
		return;
	}

	if (bComboWindowOpen)
	{
		// 윈도우 안: 즉시 다음 섹션 점프.
		if (!AdvanceToNextComboSection(InputPayload))
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer combo jump failed: CurrentComboIndex=%d", CurrentComboIndex);
		}
		return;
	}

	if (bComboGraceWindowOpen || WasInputWithinComboGrace(InputPayload))
	{
		// Grace 안에 들어온 입력: 즉시 다음 섹션 점프.
		if (!AdvanceToNextComboSection(InputPayload))
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer combo grace jump failed: CurrentComboIndex=%d", CurrentComboIndex);
		}
		return;
	}

	// 윈도우가 아직 열리기 전이면 receive buffer 에 적재.
	BufferComboInput(InputPayload);
}

void UBlackoutGA_MeleePlayer::ScheduleSectionTimers(int32 SectionIndex)
{
	if (!MeleeComboData || !MeleeComboData->ComboSections.IsValidIndex(SectionIndex))
	{
		return;
	}

	const FBlackoutComboSectionDef& Section = MeleeComboData->ComboSections[SectionIndex];

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(ComboWindowOpenTimerHandle);
	TimerManager.ClearTimer(ComboWindowCloseTimerHandle);
	TimerManager.ClearTimer(ComboGraceCloseTimerHandle);
	TimerManager.ClearTimer(ComboInputBufferTimerHandle);

	bComboWindowOpen = false;
	bComboGraceWindowOpen = false;
	ComboWindowOpenedServerTime = 0.f;
	ComboWindowClosedServerTime = 0.f;
	ActiveComboGraceDuration = 0.f;
	CurrentSectionEnteredServerTime = GetCurrentServerTimeSeconds();

	const float OpenDelay = FMath::Max(0.f, Section.WindowOpenAtSeconds);
	const float CloseDelay = FMath::Max(OpenDelay, Section.WindowCloseAtSeconds);

	if (OpenDelay <= KINDA_SMALL_NUMBER)
	{
		OnComboWindowOpenTimer();
	}
	else
	{
		TimerManager.SetTimer(
			ComboWindowOpenTimerHandle,
			this,
			&UBlackoutGA_MeleePlayer::OnComboWindowOpenTimer,
			OpenDelay,
			false);
	}

	if (CloseDelay > KINDA_SMALL_NUMBER)
	{
		TimerManager.SetTimer(
			ComboWindowCloseTimerHandle,
			this,
			&UBlackoutGA_MeleePlayer::OnComboWindowCloseTimer,
			CloseDelay,
			false);
	}

	BO_LOG_GAS(Verbose,
		"GA_MeleePlayer scheduled section timers: Index=%d Section=%s Open=%.3f Close=%.3f",
		SectionIndex,
		*Section.SectionName.ToString(),
		OpenDelay,
		CloseDelay);
}

void UBlackoutGA_MeleePlayer::OnComboWindowOpenTimer()
{
	bComboWindowOpen = true;
	bComboGraceWindowOpen = false;
	ComboWindowOpenedServerTime = GetCurrentServerTimeSeconds();
	ComboWindowClosedServerTime = 0.f;
	ActiveComboGraceDuration = 0.f;

	BO_LOG_GAS(Log,
		"GA_MeleePlayer combo window opened: CurrentComboIndex=%d Authority=%s",
		CurrentComboIndex,
		HasAuthority(&CurrentActivationInfo) ? TEXT("true") : TEXT("false"));

	// 윈도우 열리기 전에 버퍼된 입력이 있으면 즉시 흡수.
	if (bComboInputQueued)
	{
		if (IsBufferedComboInputStillValid())
		{
			if (!AdvanceToNextComboSection(QueuedComboInputPayload))
			{
				BO_LOG_GAS(Warning, "GA_MeleePlayer buffered combo jump failed: CurrentComboIndex=%d", CurrentComboIndex);
			}
		}
		else
		{
			bComboInputQueued = false;
			bHasQueuedComboInputPayload = false;
			BO_LOG_GAS(Log, "GA_MeleePlayer buffered combo input expired before window open");
		}
	}
}

void UBlackoutGA_MeleePlayer::OnComboWindowCloseTimer()
{
	if (!bComboWindowOpen)
	{
		return;
	}

	bComboWindowOpen = false;
	ComboWindowClosedServerTime = GetCurrentServerTimeSeconds();

	if (bComboInputQueued)
	{
		// 윈도우 닫히기 직전에 들어와서 buffer 에 남아 있던 입력이 있으면 즉시 점프.
		if (!AdvanceToNextComboSection(QueuedComboInputPayload))
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer combo jump at window close failed: CurrentComboIndex=%d", CurrentComboIndex);
		}
		return;
	}

	// late grace 시작.
	ActiveComboGraceDuration = GetDynamicComboGraceDuration();
	if (ActiveComboGraceDuration <= 0.f)
	{
		BO_LOG_GAS(Log, "GA_MeleePlayer combo window closed without grace");
		return;
	}

	bComboGraceWindowOpen = true;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ComboGraceCloseTimerHandle);
		TimerManager.SetTimer(
			ComboGraceCloseTimerHandle,
			this,
			&UBlackoutGA_MeleePlayer::OnComboGraceCloseTimer,
			ActiveComboGraceDuration,
			false);
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer combo grace started: Duration=%.3f", ActiveComboGraceDuration);
}

void UBlackoutGA_MeleePlayer::OnComboGraceCloseTimer()
{
	bComboGraceWindowOpen = false;
	bComboInputQueued = false;
	bHasQueuedComboInputPayload = false;
	BO_LOG_GAS(Log, "GA_MeleePlayer combo grace closed");
}

bool UBlackoutGA_MeleePlayer::AdvanceToNextComboSection(const FBlackoutAbilityInputSyncPayload& InputPayload)
{
	if (!MeleeComboData)
	{
		return false;
	}

	// 순환 콤보 계산: 마지막 인덱스 이후 다시 0으로 순환하도록 모듈러 연산 적용
	const int32 NextComboIndex = (CurrentComboIndex + 1) % MeleeComboData->ComboSections.Num();
	if (!MeleeComboData->ComboSections.IsValidIndex(NextComboIndex))
	{
		bComboInputQueued = false;
		bHasQueuedComboInputPayload = false;
		return false;
	}

	const FBlackoutComboSectionDef& NextSection = MeleeComboData->ComboSections[NextComboIndex];
	if (NextSection.SectionName == NAME_None
		|| MeleeComboData->MeleeMontage->GetSectionIndex(NextSection.SectionName) == INDEX_NONE)
	{
		bComboInputQueued = false;
		BO_LOG_GAS(Warning,
			"GA_MeleePlayer next combo section invalid: Index=%d Section=%s",
			NextComboIndex,
			*NextSection.SectionName.ToString());
		return false;
	}

	if (!HasAuthority(&CurrentActivationInfo))
	{
		BO_LOG_GAS(Verbose, "GA_MeleePlayer advance ignored on client: 서버 권위 전환 대기");
		return false;
	}

	// 서버 권위: ASC::CurrentMontageJumpToSection 이 RepAnimMontageInfo 를 갱신하여
	// 시뮬레이트 프록시 / 오너 클라이언트로 자동 전파합니다.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer advance failed: ASC가 비어 있음");
		return false;
	}

	SnapToControlYaw(&InputPayload);
	AbilitySystemComponent->CurrentMontageJumpToSection(NextSection.SectionName);
	if (ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		PlayerCharacter->Client_JumpMontageToSection(
			MeleeComboData->MeleeMontage,
			NextSection.SectionName,
			InputPayload.bHasControlYaw,
			InputPayload.ControlYawDegrees);
	}

	CurrentComboIndex = NextComboIndex;
	bComboInputQueued = false;
	bHasQueuedComboInputPayload = false;

	BO_LOG_GAS(Log,
		"GA_MeleePlayer advanced to combo section: Index=%d Section=%s Authority=%s",
		CurrentComboIndex,
		*NextSection.SectionName.ToString(),
		TEXT("true"));

	// 새 섹션의 윈도우/그레이스 타이머 예약 (서버 권위).
	ScheduleSectionTimers(CurrentComboIndex);
	return true;
}

void UBlackoutGA_MeleePlayer::BufferComboInput(const FBlackoutAbilityInputSyncPayload& InputPayload)
{
	if (!MeleeComboData || MeleeComboData->ServerReceiveBufferDuration <= 0.f)
	{
		return;
	}

	bComboInputQueued = true;
	bHasQueuedComboInputPayload = true;
	QueuedComboInputPayload = InputPayload;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ComboInputBufferTimerHandle);
		TimerManager.SetTimer(
			ComboInputBufferTimerHandle,
			this,
			&UBlackoutGA_MeleePlayer::OnComboInputBufferExpired,
			MeleeComboData->ServerReceiveBufferDuration,
			false);
	}

	BO_LOG_GAS(Log, "GA_MeleePlayer combo input buffered: CurrentComboIndex=%d", CurrentComboIndex);
}

void UBlackoutGA_MeleePlayer::OnComboInputBufferExpired()
{
	if (!bComboWindowOpen && !bComboGraceWindowOpen)
	{
		bComboInputQueued = false;
		bHasQueuedComboInputPayload = false;
		BO_LOG_GAS(Log, "GA_MeleePlayer combo input buffer expired");
	}
}

UBlackoutGA_MeleePlayer* UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(const AActor* OwnerActor)
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
			if (UBlackoutGA_MeleePlayer* MeleeAbility = Cast<UBlackoutGA_MeleePlayer>(AbilityInstance))
			{
				return MeleeAbility;
			}
		}
	}

	return nullptr;
}

void UBlackoutGA_MeleePlayer::HandleMeleeAttackWindowBegin()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer attack window begin");

	const ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;

	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		const FGameplayEffectSpecHandle DamageSpecHandle = BuildDamageSpec();
		if (!DamageSpecHandle.IsValid())
		{
			BO_LOG_GAS(Warning, "GA_MeleePlayer attack window begin skipped: 근접 데미지 스펙 생성 실패");
			return;
		}
		CombatComponent->BeginMeleeAttackWindow(DamageSpecHandle);
	}
}

void UBlackoutGA_MeleePlayer::HandleMeleeAttackWindowTick()
{
	const ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;

	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		CombatComponent->UpdateMeleeAttackWindow();
	}
}

void UBlackoutGA_MeleePlayer::HandleMeleeAttackWindowEnd()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer attack window end");

	const ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;

	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		CombatComponent->EndMeleeAttackWindow();
	}
}

void UBlackoutGA_MeleePlayer::HandleComboWindowOpened()
{
	// v2: 콤보 상태머신 권위에서 분리. 시각 effect 트리거가 필요한 경우 여기서 처리하세요.
	BO_LOG_GAS(Verbose, "GA_MeleePlayer combo window notify (FX-only, v2)");
}

void UBlackoutGA_MeleePlayer::HandleComboWindowClosed()
{
	// v2: 콤보 상태머신 권위에서 분리.
	BO_LOG_GAS(Verbose, "GA_MeleePlayer combo window close notify (FX-only, v2)");
}

void UBlackoutGA_MeleePlayer::OnMontageCompleted()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer montage completed");
	K2_EndAbility();
}

void UBlackoutGA_MeleePlayer::OnMontageInterrupted()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer montage interrupted");
	K2_EndAbility();
}

void UBlackoutGA_MeleePlayer::OnMontageCancelled()
{
	BO_LOG_GAS(Log, "GA_MeleePlayer montage cancelled");
	K2_EndAbility();
}

void UBlackoutGA_MeleePlayer::OnMontageBlendOut()
{
	BO_LOG_GAS(Verbose, "GA_MeleePlayer montage blend out");
	// blend out 시점에는 ability 를 끝내지 않습니다 — 완전 종료는 OnCompleted/Interrupted/Cancelled 에서 처리.
}

float UBlackoutGA_MeleePlayer::GetDynamicComboGraceDuration() const
{
	if (!MeleeComboData)
	{
		return 0.f;
	}

	const ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	const APlayerState* PlayerState = PlayerCharacter ? PlayerCharacter->GetPlayerState() : nullptr;
	const float PingSeconds = PlayerState ? PlayerState->GetPingInMilliseconds() * 0.001f : 0.f;
	const float GraceDuration =
		MeleeComboData->BaseComboInputGraceDuration
		+ PingSeconds * MeleeComboData->ComboInputPingScale
		+ MeleeComboData->ComboInputJitterMargin;
	return FMath::Clamp(GraceDuration, 0.f, MeleeComboData->MaxComboInputGraceDuration);
}

float UBlackoutGA_MeleePlayer::GetCurrentServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

float UBlackoutGA_MeleePlayer::GetInputServerTimeSeconds(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (!MeleeComboData)
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
		ServerReceivedTime - MeleeComboData->MaxComboInputTimestampAge,
		ServerReceivedTime + MeleeComboData->MaxComboInputFutureTolerance);
}

FBlackoutAbilityInputSyncPayload UBlackoutGA_MeleePlayer::GetLatestComboInputPayload() const
{
	if (const UBlackoutAbilitySystemComponent* BlackoutASC = Cast<UBlackoutAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		if (const FBlackoutAbilityInputSyncPayload* Payload = BlackoutASC->GetLatestInputSyncPayload(EBlackoutAbilityInputID::Melee))
		{
			return *Payload;
		}
	}

	FBlackoutAbilityInputSyncPayload FallbackPayload;
	FallbackPayload.SequenceId = 1;
	FallbackPayload.InputID = EBlackoutAbilityInputID::Melee;
	FallbackPayload.AbilitySpecHandle = GetCurrentAbilitySpecHandle();
	FallbackPayload.ClientEstimatedServerTimeSeconds = GetCurrentServerTimeSeconds();
	FallbackPayload.ServerReceivedTimeSeconds = FallbackPayload.ClientEstimatedServerTimeSeconds;
	return FallbackPayload;
}

bool UBlackoutGA_MeleePlayer::IsComboInputPayloadUsable(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (!MeleeComboData || !InputPayload.IsValid())
	{
		return false;
	}

	if (LastProcessedComboInputSequenceId != 0)
	{
		const uint16 SequenceDelta = InputPayload.SequenceId - LastProcessedComboInputSequenceId;
		if (SequenceDelta == 0 || SequenceDelta >= 32768)
		{
			BO_LOG_GAS(Verbose, "GA_MeleePlayer combo input rejected: 이미 처리한 sequence");
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

	if (RawInputServerTime > ServerReceivedTime + MeleeComboData->MaxComboInputFutureTolerance)
	{
		BO_LOG_GAS(Log, "GA_MeleePlayer combo input rejected: 미래 timestamp 허용 범위 초과");
		return false;
	}

	if (ServerReceivedTime - RawInputServerTime > MeleeComboData->MaxComboInputTimestampAge)
	{
		BO_LOG_GAS(Log, "GA_MeleePlayer combo input rejected: timestamp age 초과");
		return false;
	}

	return true;
}

bool UBlackoutGA_MeleePlayer::WasInputWithinComboGrace(const FBlackoutAbilityInputSyncPayload& InputPayload) const
{
	if (ComboWindowClosedServerTime <= 0.f || ActiveComboGraceDuration <= 0.f)
	{
		return false;
	}

	const float InputServerTime = GetInputServerTimeSeconds(InputPayload);
	return InputServerTime >= ComboWindowClosedServerTime - KINDA_SMALL_NUMBER
		&& InputServerTime <= ComboWindowClosedServerTime + ActiveComboGraceDuration;
}

bool UBlackoutGA_MeleePlayer::IsBufferedComboInputStillValid() const
{
	if (!bHasQueuedComboInputPayload || !MeleeComboData)
	{
		return true;
	}

	const float InputServerTime = GetInputServerTimeSeconds(QueuedComboInputPayload);
	const float WindowOpenTime = ComboWindowOpenedServerTime > 0.f
		? ComboWindowOpenedServerTime
		: GetCurrentServerTimeSeconds();
	const float Age = WindowOpenTime - InputServerTime;
	if (Age < -KINDA_SMALL_NUMBER)
	{
		// 입력 시각이 윈도우 오픈 시각보다 미래(클램프 후에도) → 일단 유효로 봅니다.
		return true;
	}
	return Age <= MeleeComboData->ServerReceiveBufferDuration + KINDA_SMALL_NUMBER;
}

void UBlackoutGA_MeleePlayer::ResetComboState()
{
	CurrentComboIndex = INDEX_NONE;
	bComboWindowOpen = false;
	bComboGraceWindowOpen = false;
	bComboInputQueued = false;
	bHasQueuedComboInputPayload = false;
	CurrentSectionEnteredServerTime = 0.f;
	ComboWindowOpenedServerTime = 0.f;
	ComboWindowClosedServerTime = 0.f;
	ActiveComboGraceDuration = 0.f;
	LastProcessedComboInputSequenceId = 0;
}

void UBlackoutGA_MeleePlayer::ClearAllComboTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ComboWindowOpenTimerHandle);
		TimerManager.ClearTimer(ComboWindowCloseTimerHandle);
		TimerManager.ClearTimer(ComboGraceCloseTimerHandle);
		TimerManager.ClearTimer(ComboInputBufferTimerHandle);
	}
}

void UBlackoutGA_MeleePlayer::SnapToControlYaw(const FBlackoutAbilityInputSyncPayload* InputPayload)
{
	if (ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		if (InputPayload && InputPayload->bHasControlYaw)
		{
			PlayerCharacter->SetActorRotation(FRotator(0.f, InputPayload->ControlYawDegrees, 0.f));
			return;
		}

		if (AController* Controller = PlayerCharacter->GetController())
		{
			const FRotator ControlRotation = Controller->GetControlRotation();
			PlayerCharacter->SetActorRotation(FRotator(0.f, ControlRotation.Yaw, 0.f));
		}
	}
}

FGameplayEffectSpecHandle UBlackoutGA_MeleePlayer::BuildDamageSpec() const
{
	FGameplayEffectSpecHandle SpecHandle;

	if (!MeleeComboData || !MeleeComboData->DamageEffectClass)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer BuildDamageSpec failed: DamageEffectClass 가 비어 있음");
		return SpecHandle;
	}

	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const ABOMeleeWeapon* MeleeWeapon = CombatComponent ? CombatComponent->GetMeleeWeapon() : nullptr;
	if (!MeleeWeapon)
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer BuildDamageSpec failed: 근접 무기가 없음");
		return SpecHandle;
	}

	if (!GetAbilitySystemComponentFromActorInfo())
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer BuildDamageSpec failed: ASC가 비어 있음");
		return SpecHandle;
	}

	SpecHandle = MakeOutgoingGameplayEffectSpec(MeleeComboData->DamageEffectClass, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_MeleePlayer BuildDamageSpec failed: GameplayEffectSpec 생성 실패");
		return SpecHandle;
	}

	float FinalDamage = MeleeWeapon->GetBaseDamage();
	if (MeleeComboData && MeleeComboData->ComboSections.IsValidIndex(CurrentComboIndex))
	{
		FinalDamage *= MeleeComboData->ComboSections[CurrentComboIndex].DamageMultiplier;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, FinalDamage);
	SpecHandle.Data->AddDynamicAssetTag(BlackoutGameplayTags::Kill_Melee);
	return SpecHandle;
}
