#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutEnemyAnimInstance.h"
#include "BlackoutBossAnimInstance.generated.h"

/**
 * 보스 몬스터 전용 애니메이션 인스턴스.
 * 페이즈 전환, 기믹 애니메이션 등 보스만의 특수 처리를 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutBossAnimInstance : public UBlackoutEnemyAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void UpdateAnimationProperties() {}
};