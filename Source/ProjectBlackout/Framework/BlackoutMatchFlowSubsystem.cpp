// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutMatchFlowSubsystem.h"

EBossType UBlackoutMatchFlowSubsystem::GetCurrentBossType() const
{
	return CurrentStageIndex == 0 ? EBossType::Mid : EBossType::Main;
}
