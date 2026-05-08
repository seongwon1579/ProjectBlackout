#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_SwapWeapon.generated.h"

/**
 * 플레이어 무기 교체 GA.
 * 재장전 등 액션 우선순위는 GAS 태그 정책으로 처리하고, 실제 교체 절차는 CombatComponent에 위임합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_SwapWeapon : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_SwapWeapon();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
