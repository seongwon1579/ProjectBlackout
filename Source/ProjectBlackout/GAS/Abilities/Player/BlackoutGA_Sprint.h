// ─── 구현 내역 ───────────────────────
//  - 김민영: 질주 GA — 스태미나 소모/자동 회복, 이동속도·질주 배율 데이터 기반 통합
//  - 허혁: 스프린트 클라이언트 예측 이동으로 끊김 완화
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Sprint.generated.h"

/**
 * 입력 유지 동안 이동속도를 높이고 스태미나를 주기적으로 소모하는 플레이어 전력 질주 GA.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Sprint : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Sprint();

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float MinActivationStamina = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float StaminaDrainPerTick = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float StaminaDrainInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float SprintSpeedMultiplier = 1.5f;

private:
	UFUNCTION()
	void HandleSprintTick();

	bool ShouldDrainSprintStamina() const;
	void ApplySprintSpeed(const FGameplayAbilityActorInfo* ActorInfo);
	void RestoreWalkSpeed(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool ConsumeSprintStamina() const;

	FTimerHandle SprintDrainTimerHandle;
	float CachedWalkSpeed = 0.0f;
};
