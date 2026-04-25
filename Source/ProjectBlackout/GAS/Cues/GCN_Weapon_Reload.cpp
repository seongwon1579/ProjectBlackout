#include "GAS/Cues/GCN_Weapon_Reload.h"

UGCN_Weapon_Reload::UGCN_Weapon_Reload()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Reload"));
}

bool UGCN_Weapon_Reload::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// TODO: 장전 사운드 재생, 탄창 이펙트 등 구현
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
