// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager GA 베이스 — Template 실행 구조, 현재 타겟 캐싱, 실행 중 어빌리티 태그 부여, 유효성 체크, PostActivateAbility
//  - 김민영: 보스 GA 네트워크 정책/설계 정합성, 디버그 출력 토글
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Data/BORavagerPatternData.h"
#include "GAS/Abilities/BlackoutEnemyGameplayAbility.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Ravager_Base.generated.h"

class ABORavagerBoss;
class ABlackoutBossCharacter;
class UAnimMontage;

UCLASS(Abstract)
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Base : public UBlackoutEnemyGameplayAbility
{
	GENERATED_BODY()
public:
	UBlackoutGA_Ravager_Base();

	UFUNCTION(BlueprintPure, Category = "Blackout|Debug")
	bool IsBossDebugEnabled() const { return bEnableBossDebug; }
	
protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void PostActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void SetupEventListeners() {}
	virtual FGameplayTag SelectMontageTag(const FGameplayEventData* TriggerEventData) const;
	virtual bool CanActivatePattern() const;
	virtual bool HasValidSettings() const { return true; }
	virtual bool ShouldDamageTarget(AActor* Target) const;
	
	bool TryResolveMontage(const FGameplayEventData* TriggerEventData);
	USceneComponent* TrySetupMotionWarp(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	void PlayMontage();
	UAnimMontage* GetMontage(const FGameplayTag& Tag);
	
	UFUNCTION()
	virtual void OnMontageEnded();
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	FGameplayTag PatternDataTag;

	/** 보스 GA의 화면 로그와 디버그 도형 표시 여부입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bEnableBossDebug = false;
	
	UPROPERTY(Transient)
	TObjectPtr<UBORavagerPatternData> CachedPatternData;
	
	UPROPERTY(Transient)
	TObjectPtr<ABORavagerBoss> CachedOwner;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<const APawn> CachedTarget;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> SelectedMontage;
	
	static const FName WarpTargetName;
};
