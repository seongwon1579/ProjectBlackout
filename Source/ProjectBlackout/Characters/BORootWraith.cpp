#include "Characters/BORootWraith.h"
#include "Combat/Weapons/BOProjectile.h"

ABORootWraith::ABORootWraith()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABORootWraith::FireTwinArrows()
{
	// TODO: 투사체 2연사 로직 (애니메이션 몽타주 호출 등)
}

void ABORootWraith::TeleportOutOfSight()
{
	// TODO: 점멸 이펙트 후 시야 밖으로 위치 이동
}
