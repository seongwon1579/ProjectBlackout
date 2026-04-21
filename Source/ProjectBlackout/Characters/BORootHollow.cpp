#include "Characters/BORootHollow.h"

ABORootHollow::ABORootHollow()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABORootHollow::PerformCharge()
{
	// TODO: 돌진 애니메이션 및 로직 수행
}

void ABORootHollow::OnDeath()
{
	Super::OnDeath();
	// TODO: RootHollow 전용 사망 로직 (풀 반환 등)
}
