#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Core/BlackoutTypes.h"
#include "TimerManager.h"
#include "BlackoutAbilitySystemComponent.generated.h"

class UGameplayEffect;
class UBOConsumableData;

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
	void ApplyHealthRegenOverTime(float HealAmountPerTick, float Duration, float TickInterval);

	float GetStaminaCostMultiplier() const { return StaminaCostMultiplier; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	float StaminaRegenDelay = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	float StaminaRegenTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Stamina")
	TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;

private:
	void StartStaminaRegen();
	void HandleStaminaRegenTick();
	void StopStaminaRegen();
	bool CanRecoverStamina() const;
	void ClearStaminaCostMultiplier();
	void HandleHealthRegenTick();
	void StopHealthRegen();

	FTimerHandle StaminaRegenDelayTimerHandle;
	FTimerHandle StaminaRegenTickTimerHandle;
	FTimerHandle StaminaCostMultiplierTimerHandle;
	FTimerHandle HealthRegenTimerHandle;

	UPROPERTY(Replicated)
	float StaminaCostMultiplier = 1.0f;

	float HealthRegenAmountPerTick = 0.0f;
	int32 RemainingHealthRegenTickCount = 0;
};
