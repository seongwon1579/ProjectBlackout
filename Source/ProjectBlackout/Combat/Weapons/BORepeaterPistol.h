#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOFirearm.h"
#include "BORepeaterPistol.generated.h"

/**
 * 테스트 및 초기 플레이어 무장 검증용 리피터 피스톨.
 * 메시/사운드/VFX 에셋이 없어도 사격 GA, 탄약, 장전 흐름을 확인할 수 있도록 기본 스탯만 제공합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORepeaterPistol : public ABOFirearm
{
	GENERATED_BODY()

public:
	ABORepeaterPistol();
};
