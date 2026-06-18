// ─── 구현 내역 ───────────────────────
//  - 김민영: 방향 입력 기반 구르기/연속 체인 회피, 서버 권위 체인 윈도우 + 로컬 예측 동기화, 캔슬 윈도우 연계
//  - 허혁: 백스텝 회피, 다운드/피격 상태 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "BlackoutGA_Dodge.generated.h"

class UAnimMontage;
class ABlackoutPlayerCharacter;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitGameplayEvent;
class UBODodgeData;

/**
 * 방향 입력에 따라 구르기/연속 구르기를 수행하는 플레이어 회피 GA (TDD v5 §4.1 v2).
 *
 * v2 권위 모델:
 * - 체인 윈도우 / 재시작은 **서버 권위**. 클라이언트는 입력만 표준 GAS 복제 이벤트로 전파.
 * - 몽타주 재생/재시작은 `UAbilityTask_PlayMontageAndWait` + ASC RepAnimMontageInfo 자동 복제.
 *   `Multicast_PlayDodgeMontage` 는 v2 에서 폐기.
 * - 체인 윈도우는 `UBODodgeData::ChainWindow*AtSeconds` 데이터 + 서버 World Time 타이머.
 * - AnimNotify (`BOAnimNotify_DodgeChainWindowOpen`) 는 시각 effect 보조 전용. 권위에는 관여 X.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Dodge : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Dodge();

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	/**
	 * 활성 사전 조건 검사. 데이터/스태미나 부족 시 false 를 반환하면 GAS 가
	 * 서버에서 ClientActivateAbilityFailed RPC 를 자동 호출하여 클라 예측을 폐기합니다.
	 * 이 덕분에 클라가 잘못 시작한 GA 가 몽타주 종료 시 강제 워프되는 현상이 사라집니다.
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** 현재 액터에서 활성 중인 회피 어빌리티 인스턴스를 찾습니다. */
	static UBlackoutGA_Dodge* GetActiveDodgeAbilityFromActor(const AActor* OwnerActor);

	/**
	 * 회피 체인 입력 허용 구간 시작 노티파이에서 호출됩니다.
	 * v2: 콤보 상태머신 권위에서 분리되었으며, 시각 effect 트리거가 필요할 때만 사용합니다.
	 */
	void HandleChainWindowOpened();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 회피 데이터 자산. 몽타주/모션 강도/체인 윈도우/버퍼/그레이스/timestamp 클램프 수치 포함. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dodge")
	TObjectPtr<UBODodgeData> DodgeData;

private:
	UFUNCTION()
	void OnDodgeMontageCompleted();

	UFUNCTION()
	void OnDodgeMontageInterrupted();

	UFUNCTION()
	void OnDodgeMontageCancelled();

	UFUNCTION()
	void OnDodgeMontageBlendOut();

	bool StartMontageTask(UAnimMontage* MontageToPlay, FName StartSectionName);

	void StartChainInputTask();

	UFUNCTION()
	void OnChainInputPressed(float TimeWaited);

	void StartCancelableEventTask();

	UFUNCTION()
	void OnAbilityCancelableEventReceived(FGameplayEventData Payload);

	/** 캔슬 윈도우 활성화 기간 동안 프레임단위로 무브먼트(WASD) 입력 유무를 검사합니다. */
	void CheckCancelInput();

	/**
	 * 서버에서 입력을 평가하여 윈도우/그레이스/buffer 매칭을 수행합니다.
	 * 클라이언트는 입력 복제만 담당하고, 체인 재시작은 RepAnimMontageInfo 로 따라갑니다.
	 */
	void ProcessChainInput();

	/** 회피 시작 시각 기준으로 체인 윈도우/그레이스 타이머를 예약합니다. */
	void ScheduleChainTimers();

	void OnChainWindowOpenTimer();
	void OnChainWindowCloseTimer();
	void OnChainGraceCloseTimer();
	void OnChainInputBufferExpired();

	/**
	 * 서버 권위 체인 회피 시작. 스태미나 소모와 몽타주 위치 리셋은 서버에서만 수행됩니다.
	 */
	bool StartChainedDodge(const FBlackoutAbilityInputSyncPayload& InputPayload);

	/** 체인 입력을 receive buffer 에 임시 저장합니다. */
	void BufferChainInput(const FBlackoutAbilityInputSyncPayload& InputPayload);

	/** 서버: 입력 페이로드 유효성 검증. */
	bool IsChainInputPayloadUsable(const FBlackoutAbilityInputSyncPayload& InputPayload) const;
	bool WasInputWithinChainGrace(const FBlackoutAbilityInputSyncPayload& InputPayload) const;
	bool IsBufferedChainInputStillValid() const;

	float GetDynamicChainGraceDuration() const;
	float GetCurrentServerTimeSeconds() const;
	float GetInputServerTimeSeconds(const FBlackoutAbilityInputSyncPayload& InputPayload) const;
	FBlackoutAbilityInputSyncPayload GetLatestChainInputPayload() const;

	/** 회피 모션 실행: 방향 계산 → 몽타주 재생/위치 리셋 → 체인 윈도우 예약. 체인 재시작은 서버 한정입니다. */
	bool StartDodgeInternal(ABlackoutPlayerCharacter* PlayerCharacter, bool bIsChainRestart, const FBlackoutAbilityInputSyncPayload* InputPayload = nullptr);

	/**
	 * TODO(stamina-cost): TDD §4.1 v2 에서는 GE Cost 로 처리하도록 명시.
	 * 현재는 v1 의 `ApplyModToAttribute` 경로를 유지하고, 후속 PR 에서 GE 기반으로 교체합니다.
	 */
	bool CanPayStaminaCost() const;
	bool ConsumeStamina() const;

	UAnimMontage* ResolveDodgeMontage(bool bIsBackstep) const;
	FName ResolveDodgeStartSection(const UAnimMontage* MontageToPlay, bool bIsBackstep) const;
	FVector CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep, bool bPreferControlForwardWhenNoInput = false, const FBlackoutAbilityInputSyncPayload* InputPayload = nullptr) const;

	void ClearAllChainTimers();
	void ResetChainState();

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputPress> ChainInputTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> CancelableEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveDodgeMontage;

	// DodgeEndTimerHandle 는 v2 에서 제거. PlayMontageAndWait 의 OnCompleted/OnInterrupted 콜백이 GA 종료를 담당합니다.
	FTimerHandle ChainWindowOpenTimerHandle;
	FTimerHandle ChainWindowCloseTimerHandle;
	FTimerHandle ChainGraceCloseTimerHandle;
	FTimerHandle ChainInputBufferTimerHandle;
	FTimerHandle CancelInputCheckTimerHandle;

	FBlackoutAbilityInputSyncPayload QueuedChainInputPayload;
	float CurrentDodgeStartedServerTime = 0.f;
	float ChainWindowOpenedServerTime = 0.f;
	float ChainWindowClosedServerTime = 0.f;
	float ActiveChainGraceDuration = 0.f;

	bool bChainWindowOpen = false;
	bool bChainGraceWindowOpen = false;
	bool bChainInputQueued = false;
	bool bHasQueuedChainInputPayload = false;
	bool bCancelWindowOpen = false;
	bool bCurrentDodgeIsBackstep = false;
	uint16 LastProcessedChainInputSequenceId = 0;
};
