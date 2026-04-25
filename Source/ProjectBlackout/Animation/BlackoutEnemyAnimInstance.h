#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutAnimInstanceBase.h"
#include "BlackoutEnemyAnimInstance.generated.h"

class ABlackoutEnemyCharacter;

/**
 * 일반 적(미니언) 애니메이션 인스턴스의 베이스 클래스.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutEnemyAnimInstance : public UBlackoutAnimInstanceBase
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 적 캐릭터 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<ABlackoutEnemyCharacter> EnemyCharacter;

	/** 현재 공격 중인지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsAttacking;
};
