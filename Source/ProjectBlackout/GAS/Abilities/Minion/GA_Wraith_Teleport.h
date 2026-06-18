// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: Wraith 점멸 — Vanish~Appear 무적 구간, 텔포 군집 방지(RandomBest + RVO 회피), 헤드샷 히트박스 연계
//  - 김민영: 순간이동 중 체력바 유지
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameplayEffectTypes.h"
#include "GA_Wraith_Teleport.generated.h"

class UAnimMontage;
class UEnvQuery;
class UPrimitiveComponent;


/**
 * Wraith 점멸.
 * Fire 직후 또는 시야 차단 시 EQS 결과 위치로 즉시 이동. 비행 중 무적.
 */
UCLASS()
class PROJECTBLACKOUT_API
	UGA_Wraith_Teleport : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Wraith_Teleport();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo
	                             ActivationInfo,
	                             const FGameplayEventData*
	                             TriggerEventData) override;
	
	void OnEQSFinished(TSharedPtr<FEnvQueryResult> Result);
	void StartMontageAndWaitEvents();
	
	UFUNCTION()
	void OnVanishEvent(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnAppearEvent(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageEnded();
	
	void RemoveInvulnerableTag();

	/** 점멸 연출 중 체력바 위젯을 제외한 렌더 컴포넌트만 숨김 처리합니다. */
	void SetTeleportVisualsHidden(bool bHidden);
	
	UPROPERTY(EditDefaultsOnly,Category="Wraith")
	TObjectPtr<UEnvQuery> TeleportQuery;
	
	UPROPERTY(EditDefaultsOnly,Category="Wraith")
	TObjectPtr<UAnimMontage> TeleportAnimMontage;
	
	FVector CachedDestination = FVector::ZeroVector;

	/** 점멸 전 컴포넌트 숨김 상태를 저장해, 원래 숨겨져 있던 컴포넌트를 임의로 켜지 않도록 합니다. */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> PreviousTeleportHiddenStates;
};
