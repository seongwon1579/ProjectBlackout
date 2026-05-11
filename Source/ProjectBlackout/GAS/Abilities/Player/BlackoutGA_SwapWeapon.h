#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_SwapWeapon.generated.h"

class UAnimMontage;
struct FGameplayEventData;

/**
 * 플레이어 무기 교체 GA.
 * 재장전 등 액션 우선순위는 GAS 태그 정책으로 처리하고, 무기 결합 시점은 Gameplay Event Notify로 받습니다.
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
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnWeaponSwapCommitEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnWeaponSwapMontageCompleted();

	TObjectPtr<UAnimMontage> CachedWeaponSwapMontage = nullptr;
	FGameplayTag CachedTargetWeaponSlotTag;
	bool bWeaponSwapCommitted = false;
};
