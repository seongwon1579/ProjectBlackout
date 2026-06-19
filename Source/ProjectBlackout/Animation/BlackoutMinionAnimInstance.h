// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 미니언 블렌드스페이스용 속도·방향 계산과 사망 처리(OnDeath)를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutEnemyAnimInstance.h"
#include "BlackoutMinionAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutMinionAnimInstance : public UBlackoutEnemyAnimInstance
{
	GENERATED_BODY()

public:
	void OnDeath();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Direction;

	// UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	// float bIsDead = false;
};
