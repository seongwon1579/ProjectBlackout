// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Shrewd/BOGA_Shrewd_FireStraightArrow.h"
#include "BOProjectile.h"

void UBOGA_Shrewd_FireStraightArrow::LaunchProjectile(ABOProjectile* Arrow, const FVector& SpawnLocation,
                                                      const FVector& TargetLocation)
{
	if (!Arrow) return;

	const FVector Velocity = (TargetLocation - SpawnLocation).GetSafeNormal();
	Arrow->Launch(Velocity);
}
