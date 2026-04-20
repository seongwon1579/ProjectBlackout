#include "GAS/Cues/GCN_HitImpact.h"

UGCN_HitImpact::UGCN_HitImpact()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit"));
}

bool UGCN_HitImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// TODO: Parameters.Location을 활용하여 타격 위치에 혈흔 파티클 및 사운드 스폰
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
