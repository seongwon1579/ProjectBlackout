#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutEnemyAnimInstance.h"
#include "BlackoutBossAnimInstance.generated.h"

class ABlackoutBossCharacter;

/**
 * 보스 몬스터 전용 애니메이션 인스턴스.
 * 페이즈 전환, 기믹 애니메이션 등 보스만의 특수 처리를 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutBossAnimInstance : public UBlackoutEnemyAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 보스 캐릭터 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<ABlackoutBossCharacter> BossCharacter;

	/** 현재 보스 페이즈 (1, 2, 3...) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	int32 BossPhase;

	/** 그로기 상태 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsGroggy;
};
