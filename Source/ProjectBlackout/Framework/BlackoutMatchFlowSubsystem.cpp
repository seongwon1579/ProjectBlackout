// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutMatchFlowSubsystem.h"

EBossType UBlackoutMatchFlowSubsystem::GetCurrentBossType() const
{
	return CurrentStageIndex == 0 ? EBossType::Mid : EBossType::Main;
}

float UBlackoutMatchFlowSubsystem::GetBossHealthMultiplier() const
{
	if (const float* Found = BossHealthMultiplierByPlayerCount.Find(ExpectedPlayers))
	{
		return *Found;
	}
	
	ensureMsgf(false ,TEXT("BossHealthMultiplier: ExpectedPlayers=%d 미정의, 1.0 fallback"), ExpectedPlayers);
	return 1.0f;
}
