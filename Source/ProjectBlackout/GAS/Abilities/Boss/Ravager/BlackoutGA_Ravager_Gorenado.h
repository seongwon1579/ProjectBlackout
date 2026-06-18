// ─── 구현 내역 ───────────────────────
//  - 조성원: Gorenado 어빌리티 구현, 시전 중 태그 부여로 회피 루트모션 너프, 첫 데미지 선판정
//  - 김민영: Gorenado GameplayCue SFX/VFX 연동
//  - 허혁: 적중 시 플레이어 스턴 게이지 부여
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "BlackoutGA_Ravager_Gorenado.generated.h"

/**
 * Phase C — 궁극기 볼텍스 소용돌이 (중앙으로 흡입)
 */
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Gorenado : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

protected:
	virtual void SetupEventListeners() override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual bool HasValidSettings() const override;
	
	UFUNCTION()
	void OnPullStartNotify(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnPullEndNotify(FGameplayEventData Payload);
	
	void UpdatePulling();
	
	void ApplyDamage();
	
	bool IsTargetBlocked(AActor* Target) const;
	
	void PullTarget(AActor* Target, float DeltaTime);
	
	void SetBeingPulledTag(AActor* Target, bool bApply);

private:
	
	void ClearAllPulledTags();
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitBeginEvent;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEndEvent;
	
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> PulledActors;
	
	FTimerHandle UpdateTimer;
	
	FTimerHandle DamageTimer;
	
	static constexpr float UpdateInterval = 1.f / 60.f;
	
	bool CheckValid = false;;
	
};
