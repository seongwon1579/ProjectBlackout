#include "GAS/Cues/GCN_Weapon_Fire.h"

UGCN_Weapon_Fire::UGCN_Weapon_Fire()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Fire"));
}

bool UGCN_Weapon_Fire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// TODO: MyTarget에 위치한 무기의 총구에서 파티클 스폰 및 사운드 재생
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
