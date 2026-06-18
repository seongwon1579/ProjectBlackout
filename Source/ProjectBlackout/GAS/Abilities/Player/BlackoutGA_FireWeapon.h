// ─── 구현 내역 ───────────────────────
//  - 김민영: 사격 GA 본체 — 탄약 코스트/트레이스/발사체, 데미지 Spec 빌드, 산탄 펠릿·탄퍼짐·반동, 무기별 GCN
//  - 허혁: Fire 서버 판정 분리, Huntmaster 사격 후처리 및 연사 rate 조정
//  - 최승현: 매치 통계 사격 집계 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_FireWeapon.generated.h"

class UGameplayEffect;
class UAnimMontage;
class ABOFirearm;
class ABOShotgunFirearm;
struct FGameplayEventData;
struct FTimerHandle;

/**
 * 플레이어 사격 게임플레이 어빌리티 (TDD v5 §4.1)
 * 코스트 지불(탄약), 몽타주 재생, 트레이스/발사체 스폰을 처리합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_FireWeapon : public UBlackoutGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBlackoutGA_FireWeapon();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float ParallaxMaxDistance = 10000.0f;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FHitResult PerformTrace(const FVector& Start, const FVector& End);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayEffectSpecHandle BuildDamageSpec(const ABOFirearm* Firearm);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayEffectSpecHandle BuildPelletDamageSpec(const ABOShotgunFirearm* Firearm);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool ApplyAmmoCost();

	bool CanFireAtCurrentTime(ABOFirearm* Firearm, const FGameplayAbilityActorInfo* ActorInfo) const;

	void ReserveNextFireTime(ABOFirearm* Firearm, const FGameplayAbilityActorInfo* ActorInfo);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void PlayFireMontage();

	UFUNCTION()
	void OnWeaponFireStartEventReceived(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void OnFireMontageCompleted();

	TObjectPtr<UAnimMontage> CachedFireMontage = nullptr;
	bool bWeaponFireAnimationTriggered = false;
	float NextAllowedFireTimeSeconds = 0.0f;
	TWeakObjectPtr<ABOFirearm> LastFireRateGateWeapon = nullptr;
	FTimerHandle FireMontageCompletionTimerHandle;
};
