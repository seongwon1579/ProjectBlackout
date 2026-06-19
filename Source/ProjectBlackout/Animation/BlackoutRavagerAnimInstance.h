// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 블렌드스페이스용 속도·이동/조준 방향 계산을 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutBossAnimInstance.h"
#include "BlackoutRavagerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutRavagerAnimInstance : public UBlackoutBossAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void UpdateAnimationProperties() override;

	/** 지면 이동 속도 (블렌드 스페이스 Speed 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Speed;

	/** 이동 방향 (-180 ~ 180, 블렌드 스페이스 Direction 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Direction;

	/** 이동 방향 (-180 ~ 180, 블렌드 스페이스 Direction 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float AimDirection;
	
};
