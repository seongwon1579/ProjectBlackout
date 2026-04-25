#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOFirearm.h"
#include "BOChicagoTypewriter.generated.h"

/**
 * 시카고 타자기 테스트용 주무기 클래스.
 * 전투 컴포넌트의 주무기 장착, 사격, 재장전, 탄약 UI 흐름을 검증하기 위한 기본 총기입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABOChicagoTypewriter : public ABOFirearm
{
	GENERATED_BODY()

public:
	ABOChicagoTypewriter();
};
