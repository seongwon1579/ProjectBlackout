#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/BlackoutGA_UseConsumable.h"
#include "BlackoutGA_UseGulSerum.generated.h"

class UBOConsumableData;

/**
 * 굴 세럼 사용 어빌리티.
 * 공통 소모품 사용 절차 이후 임시 스태미나 소비 배율을 적용합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBlackoutGA_UseGulSerum : public UBlackoutGA_UseConsumable
{
	GENERATED_BODY()

protected:
	virtual bool ApplyConsumableEffect(const UBOConsumableData* UsedConsumableData) override;
};
