#include "Characters/BOShrewdBoss.h"
#include "Net/UnrealNetwork.h"

ABOShrewdBoss::ABOShrewdBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsOnPlatform = false;
	bReplicates = true;
}

void ABOShrewdBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABOShrewdBoss, bIsOnPlatform);
}

void ABOShrewdBoss::EnterPlatformPhase()
{
	if (HasAuthority())
	{
		bIsOnPlatform = true;
		//OnPhaseChanged(BossPhase::Platform);
	}
}

void ABOShrewdBoss::EnterGroundPhase()
{
	if (HasAuthority())
	{
		bIsOnPlatform = false;
		//OnPhaseChanged(BossPhase::Ground);
	}
}

// void ABOShrewdBoss::OnPhaseChanged(BossPhase NewPhase)
// {
// 	Super::OnPhaseChanged(NewPhase);
//
// 	// TODO: 페이즈 변경 시 Shrewd 전용 이펙트, 애니메이션, 태그 부여 등 구현
// }
