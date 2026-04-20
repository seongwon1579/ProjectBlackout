#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_Weapon_Fire.generated.h"

/**
 * 무기 사격 시 일회성 연출(총구 화염, 사운드 등)을 담당하는 GameplayCue (TDD v5 §11)
 */
UCLASS()
class PROJECTBLACKOUT_API UGCN_Weapon_Fire : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UGCN_Weapon_Fire();

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
