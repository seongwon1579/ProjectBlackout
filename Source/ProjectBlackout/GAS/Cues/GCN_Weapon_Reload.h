#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_Weapon_Reload.generated.h"

/**
 * 무기 재장전 시 일회성 연출(탄창 탈착음 등)을 담당하는 GameplayCue (TDD v5 §11)
 */
UCLASS()
class PROJECTBLACKOUT_API UGCN_Weapon_Reload : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UGCN_Weapon_Reload();

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
