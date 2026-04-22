#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_MeleePlayer.generated.h"

class UAnimMontage;
struct FTimerHandle;

/**
 * 플레이어 근접 공격 게임플레이 어빌리티 (TDD v5 §4.1)
 * 몽타주 재생, AnimNotify 수신을 통한 스윕 검사, 콤보 입력 윈도우 처리를 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_MeleePlayer : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_MeleePlayer();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UAnimMontage> MeleeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TMap<int32, float> ComboWindowMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float MeleeHitDelay = 0.12f;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void OnMeleeHitNotify();

	UFUNCTION()
	void OnMeleeAttackFinished();

	FTimerHandle MeleeHitTimerHandle;
	FTimerHandle MeleeFinishTimerHandle;
};
