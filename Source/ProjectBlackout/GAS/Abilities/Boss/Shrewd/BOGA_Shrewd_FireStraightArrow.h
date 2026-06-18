// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 직선 화살 발사 어빌리티
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_FireArrowBase.h"
#include "BOGA_Shrewd_FireStraightArrow.generated.h"

/**
 * 
 */
class ABOProjectile;
UCLASS()
class PROJECTBLACKOUT_API UBOGA_Shrewd_FireStraightArrow : public UBlackoutGA_Shrewd_FireArrowBase
{
	GENERATED_BODY()

protected:
	virtual void LaunchProjectile(ABOProjectile* Arrow, const FVector& SpawnLocation,
	                              const FVector& TargetLocation) override;
	
};
