#include "Characters/BORootHollow.h"

#include "MotionWarpingComponent.h"

ABORootHollow::ABORootHollow()
{
	PrimaryActorTick.bCanEverTick = false;
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}