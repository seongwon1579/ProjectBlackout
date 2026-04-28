#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOFirearm.h"
#include "BlackoutMeridian.generated.h"

/**
 * Demolition 클래스의 메리디안 유탄발사기.
 * 전용 유탄 발사체를 풀링 스폰하는 비히트스캔 보조 총기입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutMeridian : public ABOFirearm
{
	GENERATED_BODY()

public:
	ABlackoutMeridian();
};
