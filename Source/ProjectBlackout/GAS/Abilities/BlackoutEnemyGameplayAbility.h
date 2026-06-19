// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 적(보스/미니언) GA 공통 베이스 — Enemy GA 상속 계층 진입점 구성
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutEnemyGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutEnemyGameplayAbility : public UBlackoutGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBlackoutEnemyGameplayAbility();
};


