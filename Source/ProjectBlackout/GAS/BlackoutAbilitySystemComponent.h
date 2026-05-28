#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Core/BlackoutTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "BlackoutAbilitySystemComponent.generated.h"

class UGameplayEffect;
class UBOConsumableData;

USTRUCT()
struct FBlackoutAbilityInputSyncPayload
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 SequenceId = 0;

	UPROPERTY()
	float ClientInputTimeSeconds = 0.0f;

	UPROPERTY()
	float ClientEstimatedServerTimeSeconds = 0.0f;

	UPROPERTY()
	float ServerReceivedTimeSeconds = 0.0f;

	UPROPERTY()
	float ControlYawDegrees = 0.0f;

	UPROPERTY()
	bool bHasControlYaw = false;

	UPROPERTY()
	FVector2D MoveInput = FVector2D::ZeroVector;

	UPROPERTY()
	bool bHasMoveInput = false;

	UPROPERTY()
	FGameplayTag InputTag;

	UPROPERTY()
	EBlackoutAbilityInputID InputID = EBlackoutAbilityInputID::None;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	UPROPERTY()
	bool bWasAbilityActive = false;

	bool IsValid() const
	{
		return InputID != EBlackoutAbilityInputID::None && SequenceId != 0;
	}
};

/**
 * 프로젝트 전용 ASC.
 * 플레이어는 ABlackoutPlayerState, 적/보스는 ACharacter 자신이 소유.
 */
UCLASS(ClassGroup = "Blackout", meta = (BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 서버 전용. CharacterData의 GrantedAbilities 배열을 순회해 ASC에 일괄 부여.
	 * ABlackoutPlayerCharacter::PossessedBy, ABlackoutEnemyCharacter::BeginPlay 에서 호출.
	 */
	void GiveDefaultAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);

	/**
	 * 서버 전용. CharacterData의 소모품 슬롯 배열을 순회해 각 DA의 UseAbility를 ASC에 부여.
	 * 배열 0/1번은 각각 UseConsumable1/2 입력 ID에 대응합니다.
	 */
	void GiveConsumableAbilities(const TArray<TObjectPtr<UBOConsumableData>>& ConsumableSlots);

	/**
	 * 로컬 입력 ID를 기준으로 대응되는 GA를 활성화하거나 활성 어빌리티에 입력 pressed 이벤트를 전달합니다.
	 */
	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID);

	/**
	 * 로컬 입력 ID를 기준으로 활성 중인 GA에 입력 released 이벤트를 전달합니다.
	 */
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID);

	/**
	 * 서버가 마지막으로 수신한 입력 메타데이터를 반환합니다.
	 * 콤보/구르기처럼 별도 페이로드가 필요한 GA에서 보정 힌트로 사용합니다.
	 */
	const FBlackoutAbilityInputSyncPayload* GetLatestInputSyncPayload(EBlackoutAbilityInputID InputID) const;

	/**
	 * 서버 전용. 모든 GA와 GE를 제거. 미니언 풀 반환(OnReturnToPool) 시 ASC 초기화에 사용.
	 */
	void ClearAllAbilitiesAndEffects();

	/**
	 * 서버 전용. 스태미나 소비가 발생했음을 알리고 자동 회복 대기 시간을 재설정합니다.
	 */
	void NotifyStaminaSpent();

	/**
	 * 서버 전용. 굴 세럼 같은 임시 효과가 적용하는 스태미나 소비 배율입니다.
	 */
	void ApplyTemporaryStaminaCostMultiplier(float NewMultiplier, float Duration);

	/**
	 * 서버 전용. 블러드 루트 같은 소모품이 적용하는 지속 체력 회복입니다.
	 */
	void ApplyHealthRegenOverTime(float HealAmountPerTick, float Duration, float TickInterval, FGameplayTag SourceConsumableTag = FGameplayTag());

	/**
	 * 서버 전용. 다운/사망 등 회복이 더 이상 유지되면 안 되는 상태에서 지속 체력 회복을 취소합니다.
	 */
	void CancelHealthRegenOverTime();

	float GetStaminaCostMultiplier() const { return StaminaCostMultiplier; }

	/**
	 * 서버/클라 공통. 쉘터존 안에 있는 동안 true. cost 차감 가드용.
	 * 현재는 비복제 Loose 태그(State.InShelter)에 의존 — 클라 mispredict 가능.
	 * 추후 GE-with-granted-tag 전환 시 복제 정합 확보.
	 */
	bool ShouldSkipCostInShelter() const;

	/**
	 * 특정 소모품 태그에 대한 쿨다운 정보(남은 시간, 총 지속 시간)를 조회합니다.
	 * 쿨다운 진행 여부를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|GAS|Consumable")
	bool GetConsumableCooldownInfo(FGameplayTag ConsumableTag, float& OutRemainingTime, float& OutDuration) const;

	/**
	 * 클라이언트가 캔슬 윈도우 도달 시 서버에 차단 LooseTag의 즉각적인 제거를 순차적으로 강제 통보하는 Reliable 서버 RPC입니다.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RemoveLooseGameplayTag(FGameplayTag TagToRemove);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	float StaminaRegenDelay = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	float StaminaRegenTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;

private:
	UFUNCTION(Server, Reliable)
	void Server_RecordAbilityInputSyncPayload(FBlackoutAbilityInputSyncPayload Payload);

	uint16 GenerateNextInputSequence(EBlackoutAbilityInputID InputID);
	float GetEstimatedServerTimeSeconds() const;
	FBlackoutAbilityInputSyncPayload BuildInputSyncPayload(
		EBlackoutAbilityInputID InputID,
		FGameplayAbilitySpecHandle AbilitySpecHandle,
		uint16 SequenceId,
		float ClientInputTimeSeconds,
		float ClientEstimatedServerTimeSeconds,
		bool bWasAbilityActive) const;
	bool RecordAbilityInputSyncPayload(FBlackoutAbilityInputSyncPayload Payload, bool bValidateSequence);
	void NotifyActiveAbilityInputPressedFromPayload(EBlackoutAbilityInputID InputID);
	bool IsInputSequenceNewer(EBlackoutAbilityInputID InputID, uint16 NewSequenceId) const;

	void StartStaminaRegen();
	void HandleStaminaRegenTick();
	void StopStaminaRegen();
	bool CanRecoverStamina() const;
	void ClearStaminaCostMultiplier();
	void HandleHealthRegenTick();
	void StopHealthRegen();
	void ResetConsumableCooldownForTag(FGameplayTag ConsumableTag);

	UFUNCTION(Client, Reliable)
	void Client_ResetConsumableCooldown(FGameplayTag ConsumableTag);

	FTimerHandle StaminaRegenDelayTimerHandle;
	FTimerHandle StaminaRegenTickTimerHandle;
	FTimerHandle StaminaCostMultiplierTimerHandle;
	FTimerHandle HealthRegenTimerHandle;

	TMap<int32, uint16> LocalInputSequenceByID;
	TMap<int32, uint16> LastServerInputSequenceByID;
	TMap<int32, FBlackoutAbilityInputSyncPayload> LatestInputSyncPayloadByID;

	UPROPERTY(Replicated)
	float StaminaCostMultiplier = 1.0f;

	float HealthRegenAmountPerTick = 0.0f;
	int32 RemainingHealthRegenTickCount = 0;
	FGameplayTag ActiveHealthRegenSourceTag;
};
