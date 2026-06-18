// ─── 구현 내역 ───────────────────────
//  - 김민영: 재장전 GA — 탄약 보충 Notify 처리, 재장전 취소 및 사격 차단 연계
//  - 허혁: 리로드 태그 매핑/상체 슬롯 구조, 무기별 재장전 애니, 재장전 중 회피 방어 코드
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Reload.generated.h"

class UGameplayEffect;
class UAnimMontage;
struct FTimerHandle;
struct FGameplayEventData;

/**
 * 플레이어 무기 장전 게임플레이 어빌리티 (TDD v5 §4.1)
 * 장전 몽타주 재생 및 탄창 결합 Notify 시점에 ExecCalc_Reload를 통해 탄약 갱신을 수행합니다.
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
	UAnimMontage* ResolveReloadMontage(const class ABlackoutPlayerCharacter* PlayerCharacter, const class ABOFirearm* EquippedFirearm) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> ReloadEffectClass;

	UFUNCTION()
	void OnWeaponReloadStartEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnReloadAmmoCommitEventReceived(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void OnReloadMontageCompleted();

	void ApplyReloadEffect();

	FGameplayTag PendingWeaponSlotTag;
	TObjectPtr<UAnimMontage> CachedReloadMontage = nullptr;
	bool bWeaponReloadAnimationTriggered = false;
	bool bReloadEffectApplied = false;
	FTimerHandle ReloadCompletionTimerHandle;
};
