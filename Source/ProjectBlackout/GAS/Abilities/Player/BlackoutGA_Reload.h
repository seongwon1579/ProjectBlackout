#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Reload.generated.h"

class UGameplayEffect;
class UAnimMontage;

/**
 * 플레이어 무기 장전 게임플레이 어빌리티 (TDD v5 §4.1)
 * 장전 몽타주 재생 및 완료 시 ExecCalc_Reload를 통해 탄약 갱신을 수행합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Reload : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Reload();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> ReloadEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void OnReloadMontageCompleted();

	FGameplayTag PendingWeaponSlotTag;
};
