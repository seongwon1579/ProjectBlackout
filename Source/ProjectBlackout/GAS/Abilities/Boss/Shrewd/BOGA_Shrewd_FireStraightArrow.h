// Fill out your copyright notice in the Description page of Project Settings.

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout")
	float StraightSpeed = 3000.f;
};
