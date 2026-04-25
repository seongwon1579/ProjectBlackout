#include "Characters/BORavagerBoss.h"

ABORavagerBoss::ABORavagerBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	AnimPlayRateMultiplier = 1.0f;
	SummonedMinionCount = 0;
}

void ABORavagerBoss::EnterPhaseA()
{
	if (HasAuthority())
	{
		OnPhaseChanged(EBossPhase::PhaseA);
	}
}

void ABORavagerBoss::EnterPhaseB()
{
	if (HasAuthority())
	{
		OnPhaseChanged(EBossPhase::PhaseB);
	}
}

void ABORavagerBoss::EnterPhaseC()
{
	if (HasAuthority())
	{
		AnimPlayRateMultiplier = 1.3f; // TDD §6 참조, Phase C 배속 증가
		OnPhaseChanged(EBossPhase::PhaseC);
	}
}

void ABORavagerBoss::SpawnMinionWave(int32 InPhaseIdx)
{
	if (HasAuthority())
	{
		// TODO: UBlackoutPoolSubsystem을 통해 미니언 스폰 로직 구현
	}
}

void ABORavagerBoss::OnPhaseChanged(EBossPhase NewPhase)
{
	Super::OnPhaseChanged(NewPhase);

	// TODO: 페이즈별 초기화 (예: Phase B 진입 시 Enrage 이펙트 등)
}
