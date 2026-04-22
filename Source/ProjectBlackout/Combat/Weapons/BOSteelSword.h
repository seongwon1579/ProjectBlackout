#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOMeleeWeapon.h"
#include "BOSteelSword.generated.h"

/**
 * 테스트 및 초기 플레이어 근접 공격 검증용 강철 검.
 * 무기 스윕과 근접 GA 흐름을 확인할 수 있도록 기본 근접 스탯을 제공합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABOSteelSword : public ABOMeleeWeapon
{
	GENERATED_BODY()

public:
	ABOSteelSword();
};
