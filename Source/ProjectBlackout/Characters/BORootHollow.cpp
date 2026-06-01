#include "Characters/BORootHollow.h"

#include "MotionWarpingComponent.h"

ABORootHollow::ABORootHollow()
{
	PrimaryActorTick.bCanEverTick = true;
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ABORootHollow::PerformCharge()
{
	// TODO: 돌진 애니메이션 및 로직 수행
}
