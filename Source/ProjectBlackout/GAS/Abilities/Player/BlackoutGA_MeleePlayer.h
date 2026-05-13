#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "BlackoutGA_MeleePlayer.generated.h"

class UAnimMontage;
class UAnimInstance;
class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputPress;
class UBOMeleeComboData;

/**
 * 플레이어 근접 공격 게임플레이 어빌리티 (TDD v5 §4.1 v2).
 *
 * v2 권위 모델 핵심:
 * - 콤보 윈도우 / 섹션 진행은 **서버 권위**. 클라이언트는 입력만 표준 GAS 복제 이벤트로 전파.
 * - 몽타주는 `UAbilityTask_PlayMontageAndWait` + ASC `CurrentMontageJumpToSection` 으로 재생/전환.
 *   `FRepAnimMontageInfo` 가 시뮬레이트 프록시에 자동 복제하므로 별도 Multicast 사용 X.
 * - 콤보 윈도우는 데이터 자산(`UBOMeleeComboData::ComboSections`)의 절대 시각 정의 + 서버 World Time 타이머.
 * - AnimNotifyState 는 히트박스 활성/비활성에만 사용. 콤보 상태머신에는 관여하지 않음.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_MeleePlayer : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_MeleePlayer();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/**
	 * 활성 사전 조건 검사. 데이터 누락/섹션 누락 시 false 반환으로 클라 예측을 자동 폐기시킵니다.
	 * 서버 ClientActivateAbilityFailed RPC 가 자동 발생.
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** 현재 액터에서 활성 중인 근접 어빌리티 인스턴스를 찾습니다. */
	static UBlackoutGA_MeleePlayer* GetActiveMeleeAbilityFromActor(const AActor* OwnerActor);

	/** 근접 공격창 시작 시 노티파이 스테이트에서 호출됩니다. (히트박스 활성) */
	void HandleMeleeAttackWindowBegin();

	/** 근접 공격창 유지 중 노티파이 스테이트에서 매 프레임 호출됩니다. (히트박스 스윕) */
	void HandleMeleeAttackWindowTick();

	/** 근접 공격창 종료 시 노티파이 스테이트에서 호출됩니다. (히트박스 비활성) */
	void HandleMeleeAttackWindowEnd();

	/**
	 * 콤보 윈도우 시작 노티파이에서 호출됩니다.
	 * v2: 콤보 상태머신 권위에서 분리되었으며, 시각 effect 트리거가 필요할 때만 사용합니다.
	 */
	void HandleComboWindowOpened();

	/**
	 * 콤보 윈도우 종료 노티파이에서 호출됩니다.
	 * v2: 콤보 상태머신 권위에서 분리되었으며, 시각 effect 트리거가 필요할 때만 사용합니다.
	 */
	void HandleComboWindowClosed();

protected:
	/** 콤보 정의 데이터. 몽타주/섹션 시각/버퍼/그레이스/데미지 GE 모두 포함. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UBOMeleeComboData> MeleeComboData;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();

	void StartComboInputTask();

	UFUNCTION()
	void OnComboInputPressed(float TimeWaited);

	/**
	 * 입력을 평가하여 콤보 진행/버퍼/grace 매칭을 수행합니다.
	 * v2.1: 양쪽(서버+클라)에서 동일하게 실행. 권위 의존 동작(RepAnimMontageInfo 갱신)만 내부에서 게이트됩니다.
	 */
	void ProcessComboInput();

	/** 현재 섹션 진입 시각 기준으로 콤보 윈도우/그레이스 타이머를 예약합니다. */
	void ScheduleSectionTimers(int32 SectionIndex);

	/** 윈도우 열림 타이머 콜백. */
	void OnComboWindowOpenTimer();

	/** 윈도우 닫힘 타이머 콜백. */
	void OnComboWindowCloseTimer();

	/** grace 만료 타이머 콜백. */
	void OnComboGraceCloseTimer();

	/**
	 * 다음 섹션으로 점프. 서버는 ASC::CurrentMontageJumpToSection (RepAnimMontageInfo 자동 복제),
	 * 클라이언트는 AnimInstance::Montage_JumpToSection 로 RPC 없이 로컬 예측만 수행합니다.
	 */
	bool AdvanceToNextComboSection();

	/** 입력을 receive buffer 에 임시 저장합니다. */
	void BufferComboInput(const FBlackoutAbilityInputSyncPayload& InputPayload);

	/** receive buffer 만료 콜백. */
	void OnComboInputBufferExpired();

	/** 서버: 입력 페이로드 유효성 검증. (sequence/timestamp clamp) */
	bool IsComboInputPayloadUsable(const FBlackoutAbilityInputSyncPayload& InputPayload) const;

	/** 서버: 현재 ActiveGrace 안에 들어온 입력인지 평가. */
	bool WasInputWithinComboGrace(const FBlackoutAbilityInputSyncPayload& InputPayload) const;

	/** 서버: 버퍼된 입력이 현재 윈도우 진입 시점 기준으로 유효한지 평가. */
	bool IsBufferedComboInputStillValid() const;

	float GetDynamicComboGraceDuration() const;
	float GetCurrentServerTimeSeconds() const;
	float GetInputServerTimeSeconds(const FBlackoutAbilityInputSyncPayload& InputPayload) const;
	FBlackoutAbilityInputSyncPayload GetLatestComboInputPayload() const;
	FGameplayEffectSpecHandle BuildDamageSpec() const;
	void ResetComboState();
	UAnimInstance* GetAvatarAnimInstance() const;
	void SnapToControlYaw();
	void ClearAllComboTimers();

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputPress> ComboInputTask;

	FTimerHandle ComboWindowOpenTimerHandle;
	FTimerHandle ComboWindowCloseTimerHandle;
	FTimerHandle ComboGraceCloseTimerHandle;
	FTimerHandle ComboInputBufferTimerHandle;

	FBlackoutAbilityInputSyncPayload QueuedComboInputPayload;
	float CurrentSectionEnteredServerTime = 0.f;
	float ComboWindowOpenedServerTime = 0.f;
	float ComboWindowClosedServerTime = 0.f;
	float ActiveComboGraceDuration = 0.f;
	int32 CurrentComboIndex = INDEX_NONE;
	bool bComboWindowOpen = false;
	bool bComboGraceWindowOpen = false;
	bool bComboInputQueued = false;
	bool bHasQueuedComboInputPayload = false;
};
