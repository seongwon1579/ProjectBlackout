// ─── 구현 내역 ───────────────────────
//  - 김민영: 정조준 전용 GA 분리, 회피/근접 중 조준 연계 캔슬
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Aim.generated.h"

/**
 * 입력 유지 동안 정조준 상태를 관리하는 플레이어 GA.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Aim : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Aim();

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	bool bAimStarted = false;
};
