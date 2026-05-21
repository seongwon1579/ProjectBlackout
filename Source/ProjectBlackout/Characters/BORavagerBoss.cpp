#include "Characters/BORavagerBoss.h"


ABORavagerBoss::ABORavagerBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	AnimPlayRateMultiplier = 1.0f;
	SummonedMinionCount = 0;
}


void ABORavagerBoss::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return;
	
}
